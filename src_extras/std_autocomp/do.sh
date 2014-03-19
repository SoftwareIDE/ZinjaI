echo "Dump..."
rm -rf dump
mkdir dump
cd cppref
for A in *; do
  lynx --dump -nolist -force_html -width=1024 $A > ../dump/$A
done
cd ..
echo "Compile..."
g++ extract.cpp -o extract.bin
echo "Extract..."
rm -rf h
mkdir h
cd dump
for A in *; do
  ../extract.bin "$A" "../h"
done
cd ..

echo "Sort..."
rm -rf sorted
mkdir sorted
mkdir sorted/cpp
mkdir sorted/cpp11
mkdir sorted/stl
mkdir sorted/stl11
cd h
mv iterator algorithm deque list map queue set stack vector ../sorted/stl
mv array forward_list unordered_map unordered_set ../sorted/stl11
mv typeindex type_traits chrono initializer_list tuple scoped_allocator cstdint cinttypes system_error cuchar random ratio cfenv codecvt regex atomic thread mutex shared_mutex future condition_variable ccomplex ctgmath cstdalign cstdbool ../sorted/cpp11
mv cassert bitset cctype cerrno cfloat ciso646 climits clocale cmath codectv complex csetjmp csignal cstdarg cstddef cstdio cstdlib cstring ctime cwchar cwctype dynarray ../sorted/cpp
mv exception fstream functional iomanip ios iosfwd iostream istream limits locale memory new numeric ostream sstream stdexcept streambuf string strstream typeinfo utility valarray ../sorted/cpp
cd ..


#cd sorted
#for a in $(find -name "*.html"); do mv "$a" "$(echo "$a"|sed 's/.html/.h/')"; done

#echo "Cat..."
#mkdir done
#cat sorted/cpp/* > done/cpp
#cat sorted/cpp11/* > done/cpp11
#cat sorted/stl/* > done/stl
#cat sorted/stl11/* > done/stl11

echo "Done!"