/* =========================================================================================

   This is an auto-generated file: Any edits you make may be overwritten!

*/

#ifndef BINARYDATA_H_110702986_INCLUDED
#define BINARYDATA_H_110702986_INCLUDED

namespace BinaryData
{
    extern const char*   noise_png;
    const int            noise_pngSize = 5227;

    extern const char*   saw_png;
    const int            saw_pngSize = 2773;

    extern const char*   sine_png;
    const int            sine_pngSize = 3747;

    extern const char*   square_png;
    const int            square_pngSize = 1115;

    extern const char*   triangle_png;
    const int            triangle_pngSize = 13262;

    extern const char*   main_png;
    const int            main_pngSize = 599;

    // Points to the start of a list of resource names.
    extern const char* namedResourceList[];

    // Number of elements in the namedResourceList array.
    const int namedResourceListSize = 6;

    // If you provide the name of one of the binary resource variables above, this function will
    // return the corresponding data and its size (or a null pointer if the name isn't found).
    const char* getNamedResource (const char* resourceNameUTF8, int& dataSizeInBytes) throw();
}

#endif
