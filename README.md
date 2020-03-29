# elf-packer
Simple PoC packer written for ELF binaries.
 
The way it works is simple and the technique is anything but novel.

The packer consists of at a minimum of 2 components, with an additional 3rd component being the, theoretically, user-provided payload file (in this project, this is the hw.c file which is just a hello world program). We have a packer, and we have a stub. The packer takes the stub and the payload, and stitches them together after encrpyting the payload. The stub simply takes the payload, decrypts it and then runs the file. 

A few shortcuts have been taken in this project due to the fact that I have no intention of writing an actual packer/crypter and this is purely for demonstration and for a bit of entertainment.
