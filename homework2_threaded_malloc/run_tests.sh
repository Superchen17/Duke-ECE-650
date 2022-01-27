cd my_malloc

make clean
make

cd ../thread_tests

make clean
make MALLOC_VERSION=LOCK_VERSION
./thread_test
./thread_test_malloc_free
./thread_test_malloc_free_change_thread
./thread_test_measurement

make clean
make MALLOC_VERSION=NOLOCK_VERSION
./thread_test
./thread_test_malloc_free
./thread_test_malloc_free_change_thread
./thread_test_measurement

make clean

cd ../my_malloc

make clean

cd ..
