@echo off

if not exist "./out/" mkdir out
nasm -f win32 "./examples/helloworld.yyn.asm" -o "./out/helloworld.obj"
gcc "./out/helloworld.obj" -o "./out/helloworld.exe"
"./out/helloworld.exe"
echo Program exited with code %errorlevel%