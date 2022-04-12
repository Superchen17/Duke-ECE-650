# Rootkit - Kernel Programming
- [**sneaky_mod.c**](sneaky_mod.c), sneaky kernel module
- [**sneaky_process.c**](sneaky_process.c), sneaky kernel module wrapper process
## Development Setup
0. (Optional) if using VSCode IDE, setup the property file to include paths of all header files
```
mkdir .vscode
cp c_cpp_properties.json .vscode/
```
1. use [**build.sh**](build.sh) to generate the sneaky kernel and sneaky process
```
chmod +x build.sh
./build.sh
```
2. execute the sneaky process with root permission, press ```q``` to quit
```
sudo ./sneaky_process
```
