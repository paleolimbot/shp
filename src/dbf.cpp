#include <cpp11.hpp>
#include <sstream>
#include <memory>
#include <vector>
#include "shapefil.h"
using namespace cpp11;
namespace writable = cpp11::writable;

// The DBF file format is, in a nutshell, some header information, some field
// information, followed by the record values. Enough information is available from
// the header to make random access fast. The implementation in dbfopen.c
// is written such that iterating over records then fields minimizes IO.
// The field types 'N' (numeric) and 'C' (character) are the most common 
// (GDAL doeesn't appear to write logicals as 'L' by default), but
// 'F' (float), 'I' (integer), 'D' (date) can be in shapfiles found in
// the wild. All of these values are stored as serialized character
// sequences that don't need any information about the width or precision
// to be parsed. The exception is possibly 'L', but I don't have any files
// that use this type on which to test (according to the dBase spec, this is
// 0x00 for False and 0x01 for True).
//
// This file is contains (1) a small wrapper around DBFOpen() and DBFClose()
// to manage the lifecycle of the underlying C struct, (2) a set of 
// readr-style "collectors" that assign a row/column index to an R vector,
// and (3) exported functions used by read_dbf() and read_dbf_meta() in R.

typedef struct {
    char name[12];
    char type;
    DBFFieldType dbf_type;
    int width;
    int precision;
} dbf_field_info_t;

class DBFFile {
public:
    DBFFile(std::string filename): filename(filename), hDBF(nullptr) {
        hDBF = DBFOpen(filename.c_str(), "rb");
        if (hDBF == nullptr) {
            std::stringstream err;
            err << "Failed to open DBF file '" << filename << "'";
            stop(err.str());
        }
    }

    ~DBFFile() {
        if (hDBF != nullptr) {
            DBFClose(hDBF);
        }
    }

    int field_count() {
        return DBFGetFieldCount(hDBF);
    }

    int row_count() {
        return DBFGetRecordCount(hDBF);
    }

    dbf_field_info_t field_info(int field_index) {
        dbf_field_info_t result;
        result.type = DBFGetNativeFieldType(hDBF, field_index);
        result.dbf_type = DBFGetFieldInfo(hDBF, field_index, result.name, &result.width, &result.precision);
        return result;
    }

    bool value_is_null(int row_index, int field_index) {
        return DBFIsAttributeNULL(hDBF, row_index, field_index);
    }

    const char* value_string(int row_index, int field_index) {
        return DBFReadStringAttribute(hDBF, row_index, field_index);
    }

    int value_integer(int row_index, int field_index) {
        return DBFReadIntegerAttribute(hDBF, row_index, field_index);
    }

    double value_double(int row_index, int field_index) {
        return DBFReadDoubleAttribute(hDBF, row_index, field_index);
    }

    bool value_logical(int row_index, int field_index) {
        const char* result_ptr = DBFReadLogicalAttribute(hDBF, row_index, field_index);
        return *result_ptr != 0;
    }

private:
    std::string filename;
    DBFHandle hDBF;

    [[ noreturn ]] void throw_failed_read(int row_index, int field_index, const char* type) {
        std::stringstream err;
        err << "Failed to read" << type << " (" << 
            row_index << ", " << field_index << ") from '" << filename << "'";
        stop(err.str());
    }
};

class Problems {
public:
    void add_problem(int row, int col, const char* expected, const char* actual) {
        this->row.push_back(row);
        this->col.push_back(col);
        this->expected.push_back(expected);
        this->actual.push_back(actual);
    }

    list result() {
        writable::list result = {row, col, expected, actual};
        return result;
    }

private:
    writable::integers row;
    writable::integers col;
    writable::strings expected;
    writable::strings actual;
};

class Collector {
public:
    virtual ~Collector() {}
    virtual sexp result() { return R_NilValue; }
    virtual void put(DBFFile& dbf, Problems& problems, int row_index, int field_index) {}
};

template <class vector_t>
class VectorCollector: public Collector {
public:
    VectorCollector(int size): result_(size), i(0) {}
    sexp result() { return result_; }
protected:
    vector_t result_;
    int i;
};

class StringsCollector: public VectorCollector<writable::strings> {
public:
    StringsCollector(int size): VectorCollector<writable::strings>(size) {}
    void put(DBFFile& dbf, Problems& problems, int row_index, int field_index) {
        if (dbf.value_is_null(row_index, field_index)) {
            result_[i++] = NA_STRING;
        } else{
            result_[i++] = dbf.value_string(row_index, field_index);
        }
    }
};

class IntegersCollector: public VectorCollector<writable::integers> {
public:
    IntegersCollector(int size): VectorCollector<writable::integers>(size) {}
    void put(DBFFile& dbf, Problems& problems, int row_index, int field_index) {
        if (dbf.value_is_null(row_index, field_index)) {
            result_[i++] = NA_INTEGER;
        } else{
            result_[i++] = dbf.value_integer(row_index, field_index);
        }
    }
};

class DoublesCollector: public VectorCollector<writable::doubles> {
public:
    DoublesCollector(int size): VectorCollector<writable::doubles>(size) {}
    void put(DBFFile& dbf, Problems& problems, int row_index, int field_index) {
        if (dbf.value_is_null(row_index, field_index)) {
            result_[i++] = NA_REAL;
        } else{
            result_[i++] = dbf.value_double(row_index, field_index);
        }
    }
};

class LogicalsCollector: public VectorCollector<writable::logicals> {
public:
    LogicalsCollector(int size): VectorCollector<writable::logicals>(size) {}
    void put(DBFFile& dbf, Problems& problems, int row_index, int field_index) {
        if (dbf.value_is_null(row_index, field_index)) {
            result_[i++] = NA_LOGICAL;
        } else{
            result_[i++] = dbf.value_logical(row_index, field_index);
        }
    }
};

std::unique_ptr<Collector> get_collector_user(char spec, int row_count) {
    switch(spec) {
    case '-': return std::unique_ptr<Collector>(new Collector());
    case 'c': return std::unique_ptr<Collector>(new StringsCollector(row_count));
    case 'i': return std::unique_ptr<Collector>(new IntegersCollector(row_count));
    case 'd': return std::unique_ptr<Collector>(new DoublesCollector(row_count));
    case 'l': return std::unique_ptr<Collector>(new LogicalsCollector(row_count));
    default:
        std::stringstream err;
        err << "Can't guess collector from specification '" << spec << "'";
        stop(err.str());
    }
}

std::unique_ptr<Collector> get_collector_auto(char spec, int row_count) {
    switch(spec) {
    case 'I': return std::unique_ptr<Collector>(new IntegersCollector(row_count));
    case 'F':
    case 'N': return std::unique_ptr<Collector>(new DoublesCollector(row_count));
    case 'L': return std::unique_ptr<Collector>(new LogicalsCollector(row_count));
    default: return std::unique_ptr<Collector>(new StringsCollector(row_count));
    }
}

[[cpp11::register]]
list cpp_read_dbf_meta(std::string filename) {
    DBFFile dbf(filename);
    int field_count = dbf.field_count();

    writable::integers index(field_count);
    writable::strings name(field_count);
    writable::raws type(field_count);
    writable::integers width(field_count);
    writable::integers precision(field_count); 

    dbf_field_info_t info;
    for (int field_index = 0; field_index < field_count; field_index++) {
        info = dbf.field_info(field_index);
        index[field_index] = field_index + 1; // return R-based index
        name[field_index] = info.name;
        type[field_index] = info.type;
        width[field_index] = info.width;
        precision[field_index] = info.precision;
    }

    writable::list result = {index, name, type, width, precision};
    result.names() = {"index", "name", "type", "width", "precision"};
    return result;
}

[[cpp11::register]]
list cpp_read_dbf(std::string filename, std::string col_spec) {
    DBFFile dbf(filename);
    int field_count = dbf.field_count();
    int row_count = dbf.row_count();

    // Iterate over columns to get type information. `col_spec` here
    // can be "" (use provider types), a readr-style abbreviation with
    // exactly one character or one character per column (e.g., 
    // "cd-" for char, double, skip).
    std::vector<std::unique_ptr<Collector>> collectors(field_count);
    writable::strings names(field_count);

    if (col_spec.size() == 0) {
        for (int field_index = 0; field_index < field_count; field_index++) {
            dbf_field_info_t field_info = dbf.field_info(field_index);
            names[field_index] = field_info.name;
            collectors[field_index] = get_collector_auto(field_info.type, row_count);
        }
    } else if (col_spec.size() == 1) {
        const char* col_spec_chars = col_spec.c_str();
        for (int field_index = 0; field_index < field_count; field_index++) {
            dbf_field_info_t field_info = dbf.field_info(field_index);
            names[field_index] = field_info.name;
            collectors[field_index] = get_collector_user(col_spec_chars[0], row_count);
        }
    } else if (col_spec.size() == field_count) {
        const char* col_spec_chars = col_spec.c_str();
        for (int field_index = 0; field_index < field_count; field_index++) {
            dbf_field_info_t field_info = dbf.field_info(field_index);
            names[field_index] = field_info.name;
            collectors[field_index] = get_collector_user(col_spec_chars[field_index], row_count);
        }
    } else {
        stop(
            "Can't use col_spec with size %d for DBF with %d fields", 
            col_spec.size(), field_count
        );
    }

    // Use a Problems object to accumulate parse errors
    Problems problems;

    // Iterate over rows then columns and let the collectors handle conversion
    // to R vector values.
    for (int row_index = 0; row_index < row_count; row_index++) {
        for (int field_index = 0; field_index < field_count; field_index++) {
            collectors[field_index]->put(dbf, problems, row_index, field_index);
        }
    }

    // Assemble results as a list(). Note that "skipped" columns will be R_NilValue
    writable::list result(field_count);
    for (int field_index = 0; field_index < field_count; field_index++) {
        result[field_index] = collectors[field_index]->result();
    }

    // Need to keep the row count in case there were no columns or in case
    // all columns were skipped. In this case, a data frame with the correct
    // dimensions can be calculated.
    result.names() = names;
    result.attr("n_rows") = row_count;
    result.attr("problems") = problems.result();
    return result;
}
