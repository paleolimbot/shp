#include <cpp11.hpp>
#include <sstream>
#include <memory>
#include <clocale>
#include <cstdlib>
#include <vector>
#include "shapefil.h"

// unclear where inconv_t is defined, but an invalid
// conversion is defined as (iconv_t) -1
#include <R_ext/Riconv.h>
typedef void* iconv_t;

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
// readr-style "collectors" that assign a DBF value to an R vector,
// and (3) exported functions used by read_dbf() and read_dbf_meta() in R.
// The underlying shplib implementation uses atoi and atod to parse
// strings into doubles/ints. These functions make it difficult to
// detect parse errors. Here we use C++11's strtod and strtoi with
// a thread localizer and report parse issues via a readr-style
// 'problems' object.

// Wrapper around R's iconv
// https://github.com/wch/r-source/blob/trunk/src/main/sysutils.c#L582-L778
class IconvUTF8 {
public:
    IconvUTF8(const char* from): iconv_obj(nullptr), buffer(nullptr), buffer_size(2048) {
        iconv_obj = Riconv_open("UTF-8", from);
        if(iconv_obj == (iconv_t)(-1)) {
            stop("Can't convert from encoding '%s' to encoding '%s'", from, "UTF-8");
        }

        // share an output buffer for all conversions, reallocing as necessary
        buffer = (char*) malloc(buffer_size * sizeof(char));
        if (buffer == nullptr) {
            stop("Failed to allocate IconvUTF8 output buffer");
        }
    }

    std::string iconv(const char* bytes) {
        size_t in_bytes_left = strlen(bytes);
        ensure_buffer_has_size(in_bytes_left * 2);
        size_t result = (size_t) -1;
        size_t out_bytes_left = buffer_size;
        char* ptr_out = buffer;
        result = Riconv(iconv_obj, &bytes, &in_bytes_left, &ptr_out, &out_bytes_left);

        if ((result == ((size_t) -1)) || (in_bytes_left != 0)) {
            stop("Conversion to UTF-8 failed");
        }

        std::string output(buffer, buffer_size - out_bytes_left);
        return output;
    }

    void ensure_buffer_has_size(size_t size) {
        if (size >= buffer_size) {
            buffer = (char*) realloc(buffer, size);
            if (buffer == nullptr) {
                stop("Failed to reallocate IconvUTF8 output buffer");
            } else {
                buffer_size = size;
            }
        }
    }

    ~IconvUTF8() {
        if ((iconv_obj != nullptr) && (iconv_obj != (iconv_t)(-1))) {
            Riconv_close(iconv_obj);
        }

        if (buffer != nullptr) {
            free(buffer);
        }
    }
private:
    void* iconv_obj;
    char* buffer;
    size_t buffer_size;
};

// .dbf file encodings are found in the .cpg companion file or encoded in the
// .dbf file header. The value returned by DBFGetCodePage() will be either the
// contents of the .cpg file OR an "ldid" (possiby language driver identifier?)
// integer in the form LDID/([0-9]+), where [0-9]+ is an integer that can be
// transformed into the contents of a .cpg file based on this table:
// http://www.autopark.ru/ASBProgrammerGuide/DBFSTRUC.HTM
// https://github.com/OSGeo/gdal/blob/master/gdal/ogr/ogrsf_frmts/shape/ogrshapelayer.cpp
// For example, if DBFGetCodePage() returns 'LDID/19', this is equivalent to
// a .cpg file whose contents is 'CP932'. For details on the .cpg file value,
// see https://support.esri.com/en/technical-article/000013192
class DBFEncodings {
public:
    static int dbf_encoding_cpg_from_ldid(int ldid) {
        switch (ldid) {
        case 1: return 437;
        case 2: return 850;
        case 3: return 1252;
        case 4: return 10000;
        case 8: return 865;
        case 10: return 850;
        case 11: return 437;
        case 13: return 437;
        case 14: return 850;
        case 15: return 437;
        case 16: return 850;
        case 17: return 437;
        case 18: return 850;
        case 19: return 932;
        case 20: return 850;
        case 21: return 437;
        case 22: return 850;
        case 23: return 865;
        case 24: return 437;
        case 25: return 437;
        case 26: return 850;
        case 27: return 437;
        case 28: return 863;
        case 29: return 850;
        case 31: return 852;
        case 34: return 852;
        case 35: return 852;
        case 36: return 860;
        case 37: return 850;
        case 38: return 866;
        case 55: return 850;
        case 64: return 852;
        case 77: return 936;
        case 78: return 949;
        case 79: return 950;
        case 80: return 874;
        case 87: return DBF_ENCODING_ENC_ISO8859_1;
        case 88: return 1252;
        case 89: return 1252;
        case 100: return 852;
        case 101: return 866;
        case 102: return 865;
        case 103: return 861;
        case 104: return 895;
        case 105: return 620;
        case 106: return 737;
        case 107: return 857;
        case 108: return 863;
        case 120: return 950;
        case 121: return 949;
        case 122: return 936;
        case 123: return 932;
        case 124: return 874;
        case 134: return 737;
        case 135: return 852;
        case 136: return 857;
        case 150: return 10007;
        case 151: return 10029;
        case 200: return 1250;
        case 201: return 1251;
        case 202: return 1254;
        case 203: return 1253;
        case 204: return 1257;
        default: return -1;
        }
    }

    // `value` here is the value produced by DBFGetCodePage()
    static std::string dbf_encoding(const char* c_str) {
        if ((c_str == nullptr) || (strlen(c_str) == 0)) {
            return "";
        }
        
        std::string value(c_str);
        bool is_ldid = (value.size() > 6) && (value.substr(0, 5) == "LDID/");
        bool is_cp = (value.size() > 3) && (value.substr(0, 2) == "CP");

        int cpg_code = -1;
        if (is_ldid) {
            std::string ldid_str = value.substr(5, value.size() - 5);
            int ldid_int = atoi(ldid_str.c_str());
            cpg_code = dbf_encoding_cpg_from_ldid(ldid_int);
        } else if (is_cp) {
            cpg_code = atoi(value.substr(2, value.size() - 2).c_str());
        }

        // if atoi fails, cpg_code will be 0
        if (cpg_code == DBF_ENCODING_ENC_ISO8859_1) {
            return "ISO-8859-1";
        } else if ((cpg_code >= 437 && cpg_code <= 950) || (cpg_code >= 1250 && cpg_code <= 1258)) {
            std::stringstream out;
            out << "CP" << cpg_code;
            return out.str();
        } else if ((value.size() > 5) && (value.substr(0, 5) == "8859-")) {
            std::stringstream out;
            out << "ISO-8859-" << value.substr(5, value.size() - 5);
            return out.str();
        } else if ((value.size() > 4) && (value.substr(0, 4) == "8859")) {
            std::stringstream out;
            out << "ISO-8859-" << value.substr(4, value.size() - 4);
            return out.str();
        } else if((value.size() >= 5) && (value.substr(0, 5) == "UTF-8")) {
            return "UTF-8";
        } else if((value.size() >= 4) && (value.substr(0, 4) == "UTF8")) {
            return "UTF-8";
        } else if((value.size() >= 9) && (value.substr(0, 9) == "ANSI 1251")) {
            return "CP1251";
        } else {
            return value;
        }
    }

private:
    const static int DBF_ENCODING_ENC_ISO8859_1 = 88591;
};

typedef struct {
    char name[12];
    char type;
    DBFFieldType dbf_type;
    int width;
    int precision;
} dbf_field_info_t;

class DBFFile {
public:
    DBFFile(std::string filename, std::string encoding = ""): 
        filename_(filename), encoding_(encoding), hDBF(nullptr) {
        hDBF = DBFOpen(filename.c_str(), "rb");
        if (hDBF == nullptr) {
            std::stringstream err;
            err << "Failed to open DBF file '" << filename << "'";
            stop(err.str());
        }

        if (encoding_ == "") {
            encoding_ = DBFEncodings::dbf_encoding(DBFGetCodePage(hDBF));
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

    std::string filename() {
        return filename_;
    }

    dbf_field_info_t field_info(int field_index) {
        dbf_field_info_t result;
        result.type = DBFGetNativeFieldType(hDBF, field_index);
        result.dbf_type = DBFGetFieldInfo(hDBF, field_index, result.name, &result.width, &result.precision);
        return result;
    }

    std::string encoding() {
        return this->encoding_;
    }

    bool value_is_null(int row_index, int field_index) {
        return DBFIsAttributeNULL(hDBF, row_index, field_index);
    }

    const char* value_string(int row_index, int field_index) {
        return DBFReadStringAttribute(hDBF, row_index, field_index);
    }

private:
    std::string filename_;
    std::string encoding_;
    DBFHandle hDBF;
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
        result.names() = {"row", "col", "expected", "actual"};
        return result;
    }

private:
    writable::integers row;
    writable::integers col;
    writable::strings expected;
    writable::strings actual;
};

class ThreadLocalizer {
public:
    ThreadLocalizer() {
        char* p = std::setlocale(LC_NUMERIC, nullptr);
        if(p != nullptr) {
        this->saved_locale = p;
        }
        std::setlocale(LC_NUMERIC, "C");
    }

    ~ThreadLocalizer() {
        std::setlocale(LC_NUMERIC, saved_locale.c_str());
    }
private:
    std::string saved_locale;
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
    StringsCollector(int size, const std::string& encoding): 
        VectorCollector<writable::strings>(size), 
        iconv(encoding.c_str()), 
        encoding(encoding) {}
    
    void put(DBFFile& dbf, Problems& problems, int row_index, int field_index) {
        if (dbf.value_is_null(row_index, field_index)) {
            result_[i++] = NA_STRING;
        } else {
            const char* bytes = dbf.value_string(row_index, field_index);
            try {
                result_[i++] = iconv.iconv(bytes);
            } catch(std::exception& error) {
                result_[i++] = bytes;

                std::stringstream expected;
                expected << "A string with encoding '" << encoding << "'";
                problems.add_problem(
                    row_index, field_index, 
                    expected.str().c_str(),
                    bytes
                );
            }
        }
    }

private:
    IconvUTF8 iconv;
    const std::string& encoding;
};

class IntegersCollector: public VectorCollector<writable::integers> {
public:
    IntegersCollector(int size): VectorCollector<writable::integers>(size) {}
    void put(DBFFile& dbf, Problems& problems, int row_index, int field_index) {
        if (dbf.value_is_null(row_index, field_index)) {
            result_[i++] = NA_INTEGER;
        } else {
            const char* chars = dbf.value_string(row_index, field_index);
            char* end_char;
            long int value = std::strtol(chars, &end_char, 10);

            if (end_char != (chars + strlen(chars))) {
                problems.add_problem(row_index, field_index, "no trailing characters", chars);
                result_[i++] = NA_INTEGER;
            } else if ((value > INT_MAX) || (value < NA_INTEGER)) {
                result_[i++] = NA_INTEGER;
            } else {
                result_[i++] = value;
            }
        }
    }
};

class DoublesCollector: public VectorCollector<writable::doubles> {
public:
    DoublesCollector(int size): VectorCollector<writable::doubles>(size) {}
    void put(DBFFile& dbf, Problems& problems, int row_index, int field_index) {
        if (dbf.value_is_null(row_index, field_index)) {
            result_[i++] = NA_REAL;
        } else {
            const char* chars = dbf.value_string(row_index, field_index);
            char* end_char;
            double value = std::strtod(chars, &end_char);

            if (end_char != (chars + strlen(chars))) {
                problems.add_problem(row_index, field_index, "no trailing characters", chars);
                result_[i++] = NA_REAL;
            } else {
                result_[i++] = value;
            }
        }
    }
};

class LogicalsCollector: public VectorCollector<writable::logicals> {
public:
    LogicalsCollector(int size, char dbf_type): 
        VectorCollector<writable::logicals>(size), dbf_type(dbf_type) {}
    
    void put(DBFFile& dbf, Problems& problems, int row_index, int field_index) {
        if (dbf.value_is_null(row_index, field_index)) {
            result_[i++] = NA_LOGICAL;
        } else if (dbf_type == 'L') {
            const char* chars = dbf.value_string(row_index, field_index);
            if (strlen(chars) > 1) {
                result_[i++] = NA_LOGICAL;
            } else if (chars[0] > 1) {
                char hex_buf[5];
                sprintf(hex_buf, "%#02x", chars[0]);
                problems.add_problem(row_index, field_index, "0x00 or 0x01", hex_buf);
                result_[i++] = NA_LOGICAL;
            } else {
                result_[i++] = chars[0];
            }
        } else {
            std::string chars = dbf.value_string(row_index, field_index);
            if (chars == "true" || chars == "TRUE" || 
                chars == "T" || chars == "t" || chars == "1") {
                result_[i++] = 1;
            } else if (chars == "false" || chars == "FALSE" || 
                chars == "F" || chars == "f" || chars == "0") {
                result_[i++] = 0;
            } else {
                problems.add_problem(row_index, field_index, "true/TRUE/t/1/false/FALSE/f/0", chars.c_str());
                result_[i++] = NA_LOGICAL;
            }
        }
    }

private:
    char dbf_type;
};


class CollectorFactory {
public:
    CollectorFactory(DBFFile& dbf): dbf(dbf) {}

    std::unique_ptr<Collector> get_collector_auto(char spec, int row_count) {
        std::string encoding = dbf.encoding();
        if (encoding == "") {
            encoding = "UTF-8";
        }

        switch(spec) {
        case 'I': return std::unique_ptr<Collector>(new IntegersCollector(row_count));
        case 'F':
        case 'N': return std::unique_ptr<Collector>(new DoublesCollector(row_count));
        case 'L': return std::unique_ptr<Collector>(new LogicalsCollector(row_count, 'L'));
        default: 
            return std::unique_ptr<Collector>(new StringsCollector(row_count, encoding));
        }
    }

    std::unique_ptr<Collector> get_collector_user(char spec, int row_count, char dbf_type) {
        std::string encoding = dbf.encoding();
        if (encoding == "") {
            encoding = "UTF-8";
        }

        switch(spec) {
        case '?': return get_collector_auto(dbf_type, row_count);
        case '-': return std::unique_ptr<Collector>(new Collector());
        case 'c': return std::unique_ptr<Collector>(new StringsCollector(row_count, encoding));
        case 'i': return std::unique_ptr<Collector>(new IntegersCollector(row_count));
        case 'd': return std::unique_ptr<Collector>(new DoublesCollector(row_count));
        case 'l': return std::unique_ptr<Collector>(new LogicalsCollector(row_count, dbf_type));
        default:
            std::stringstream err;
            err << "Can't guess collector from specification '" << spec << "'";
            stop(err.str());
        }
    }

private:
    DBFFile& dbf;
};



[[cpp11::register]]
list cpp_read_dbf_colmeta(std::string filename) {
    DBFFile dbf(filename);
    int field_count = dbf.field_count();

    writable::strings name(field_count);
    writable::raws type(field_count);
    writable::integers width(field_count);
    writable::integers precision(field_count); 

    dbf_field_info_t info;
    for (int field_index = 0; field_index < field_count; field_index++) {
        info = dbf.field_info(field_index);
        name[field_index] = info.name;
        type[field_index] = info.type;
        width[field_index] = info.width;
        precision[field_index] = info.precision;
    }

    writable::list result = {name, type, width, precision};
    result.names() = {"name", "type", "width", "precision"};
    return result;
}

[[cpp11::register]]
list cpp_read_dbf(std::string filename, std::string col_spec, std::string encoding) {
    DBFFile dbf(filename, encoding);
    int field_count = dbf.field_count();
    int row_count = dbf.row_count();

    // Make sure the C locale for parsing numerics is consistent.
    // The deleter will restore the current C locale on exit.
    ThreadLocalizer localizer;

    // Iterate over columns to get names and type information.
    CollectorFactory collector_factory(dbf);
    std::vector<std::unique_ptr<Collector>> collectors(field_count);
    writable::strings names(field_count);

    if (col_spec.size() == 1) {
        const char* col_spec_chars = col_spec.c_str();
        for (int field_index = 0; field_index < field_count; field_index++) {
            dbf_field_info_t field_info = dbf.field_info(field_index);
            names[field_index] = field_info.name;
            collectors[field_index] = collector_factory.get_collector_user(
                col_spec_chars[0], 
                row_count,
                field_info.type
            );
        }
    } else if (col_spec.size() == field_count) {
        const char* col_spec_chars = col_spec.c_str();
        for (int field_index = 0; field_index < field_count; field_index++) {
            dbf_field_info_t field_info = dbf.field_info(field_index);
            names[field_index] = field_info.name;
            collectors[field_index] = collector_factory.get_collector_user(
                col_spec_chars[field_index], 
                row_count, 
                field_info.type
            );
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
        if ((row_index + 1) % 1000 == 0) {
            check_user_interrupt();
        }

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
