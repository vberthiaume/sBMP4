#!/bin/bash

VERSION="$1"

pushd C:\\Users\\Vincent\\Documents\\git\\sBMP4\\Builds\\VisualStudio2013\\Release

powershell.exe -nologo -noprofile -command "& { Add-Type -A 'System.IO.Compression.FileSystem'; [IO.Compression.ZipFile]::ExtractToDirectory('C:\\Users\\Vincent\\Desktop\\sBMP4$VERSION.zip', 'sBMP4.dll'); }"
powershell.exe -nologo -noprofile -command "& { Add-Type -A 'System.IO.Compression.FileSystem'; [IO.Compression.ZipFile]::ExtractToDirectory('C:\\Users\\Vincent\\Desktop\\sBMP4$VERSION.zip', 'sBMP4.vst3'); }"

popd

# pushd ~/Library/Audio/Plug-Ins/VST/
# zip -r ~/Desktop/Octogris$VERSION.zip ./Octogris2.vst 
# popd

#zip -rj ~/Desktop/ZirkOSC.zip ~/Library/Audio/Plug-Ins/Components/ZirkOSC3.component
#~/Library/Audio/Plug-Ins/VST/ZirkOSC3.vst 

echo "Created zip file sBMP4$VERSION.zip"
