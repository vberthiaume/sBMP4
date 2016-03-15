/* =========================================================================================

   This is an auto-generated file: Any edits you make may be overwritten!

*/

#ifndef BINARYDATA_H_110702986_INCLUDED
#define BINARYDATA_H_110702986_INCLUDED

namespace BinaryData
{
    extern const char*   After_Shok_ttf;
    const int            After_Shok_ttfSize = 59436;

    extern const char*   _2Dumb_ttf;
    const int            _2Dumb_ttfSize = 66148;

    extern const char*   _3Dumb_ttf;
    const int            _3Dumb_ttfSize = 142224;

    extern const char*   AmaticBold_ttf;
    const int            AmaticBold_ttfSize = 118784;

    extern const char*   ANUDI____ttf;
    const int            ANUDI____ttfSize = 49832;

    extern const char*   ANUDRG___ttf;
    const int            ANUDRG___ttfSize = 48560;

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
    const int namedResourceListSize = 12;

    // If you provide the name of one of the binary resource variables above, this function will
    // return the corresponding data and its size (or a null pointer if the name isn't found).
    const char* getNamedResource (const char* resourceNameUTF8, int& dataSizeInBytes) throw();
}

#endif
