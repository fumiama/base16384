# base16384
Encode binary files to readable utf16be.

# Description 说明
Use 16384 Chinene characters as the alphabet, like the 64 chars in the base64.
使用16384个汉字作为字符表，就像base64用64个字符作为字符表一样。

# Benefits 优点
Save more space and since the code 0x0000 is encoded to "一", finding zero space seems to bo easier.
节省更多空间。
更容易发现二进制文件的规律。
