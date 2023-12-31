#include "hdf5.h"
#include <stdio.h>
#include "general_io.h"

hid_t create_write_dataset(hid_t group, const char* name, hid_t datatype, hid_t dataspace, void* data, const char* unit)
{
    // Create dataset
    hid_t dataset = H5Dcreate2(group, name, datatype, dataspace, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    if (dataset < 0) {
        fprintf(stderr, "Failed to create dataset for %s\n", name);
        return H5I_INVALID_HID;
    }

    // Write data to dataset
    herr_t status = H5Dwrite(dataset, datatype, H5S_ALL, H5S_ALL, H5P_DEFAULT, data);
    if (status < 0) {
        fprintf(stderr, "Failed to write data to dataset %s\n", name);
        H5Dclose(dataset);
        return H5I_INVALID_HID;
    }

    // Add unit attribute to the dataset
    add_string_attribute(dataset, "unit", unit);

    // Close dataset
    status = H5Dclose(dataset);
    if (status < 0) {
        fprintf(stderr, "Failed to close dataset %s\n", name);
        return H5I_INVALID_HID;
    }

    return dataset;  // Return dataset id in case caller needs it (though in this context it's already closed)
}