cd backend
mkdir build
cd build
cmake ../..
make
./server &
cd ..
python3 web/app.py
