# gen_iota_address

Proof of concept, performance comparison implementation in C 

app template: https://github.com/oopsmonk/iota_c_templates


### build version
```bash
mkdir build && cd build

cmake -DCMAKE_INSTALL_PREFIX=$PWD .. && make -j8

# run the example 
./my_app

```

### Debug version
```bash
mkdir debug && cd debug
cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INSTALL_PREFIX=$PWD .. && make -j8
# run the example, you can run gdb... 
./my_app
```
