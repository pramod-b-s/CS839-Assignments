Folders hw1-reserve_publish and hw1-transactional_api contain two different implementations of the Key Value store program. The transactional API is used due to the following limitation of reserve publish API:

"The reserve/publish API is fast, but it does not allow reading data you have just written.
In such cases, you can use the transactional API."
The reserve/publish does not allow to overwrite an entry with an existing key, hence transactional API is used to get the existing key-value pair and update the value. The Get method used in reserve/publish mode also results in crash when no key-value pairs are present, probably due to the above limitation.

Basic testing has been done for the put, get methods and it is verified that with a crash of the binary, the key-value pairs still persist in the memory. The following commands can be used to test:
gcc -o pgm1 pgm1.c -lpmemobj -lpmem -pthread
./pgm1 p <key> <value>		=> store given key-value pair; update if key is present(with TxApi only) 
./pgm1 g <key> 				=> get value corresponding to key
./pgm1 d 					=> display all key-value pairs stored
./pgm1 c 					=> cause crash


Experience:

Inspite of there being a few minor issues, the PMEM library was pretty easy to install. The instructions shared on the website were really useful for installation.
For the reserve/publish methods, not much documentation is available. The PMEM book had a sample program which was helpful. The transactional API was very useful for implementing the program and it has a lot of helpful documentation available online, including the link shared on the course website.
It was exciting to see persistence in action and it is very easy to program as well. Hope the   