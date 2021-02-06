#include <cpp11.hpp>
#include <sstream>
#include "shapefil.h"
using namespace cpp11;
namespace writable = cpp11::writable;

typedef struct {
    char name[12];
    char type;
    DBFFieldType dbf_type;
    int width;
    int precision;
} dbf_field_info_t;

class DBFFile {
public:
    DBFFile(std::string filename): hDBF(nullptr) {
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

    dbf_field_info_t field_info(int i) {
        dbf_field_info_t result;
        result.type = DBFGetNativeFieldType(hDBF, i);
        result.dbf_type = DBFGetFieldInfo(hDBF, i, result.name, &result.width, &result.precision);
        return result;
    }

private:
    DBFHandle hDBF;
};

[[cpp11::register]]
list cpp_read_dbf_meta(std::string filename) {
    DBFFile dbf(filename);
    int n_fields = dbf.field_count();

    writable::integers index(n_fields);
    writable::strings name(n_fields);
    writable::raws type(n_fields);
    writable::integers width(n_fields);
    writable::integers precision(n_fields); 

    dbf_field_info_t info;
    for (int i = 0; i < dbf.field_count(); i++) {
        info = dbf.field_info(i);
        index[i] = i + 1; // return R-based index
        name[i] = info.name;
        type[i] = info.type;
        width[i] = info.width;
        precision[i] = info.precision;
    }

    writable::list result = {index, name, type, width, precision};
    result.names() = {"index", "name", "type", "width", "precision"};
    return result;
}

[[cpp11::register]]
void cpp_read_dbf(std::string filename) {
    DBFFile dbf(filename);

}
