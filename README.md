# base16384
Encode binary file to printable utf16be, and vise versa.

# Description 说明
Use 16384 Chinene characters (from \u4E00 to \u8DFF) as the "alphabet", just like the 64 chars did in the base64.

使用16384个汉字作为字符表，就像base64用64个字符作为字符表一样。

We use \u3Exx as a log of the remainder bytes of data after % 7, while xx ranging from 01 to 06.

另外，使用\u3Exx作为编码时数据不满7位的个数，范围在01~06。

# Benefits 优点
Save more space and since the code 0x0000 is encoded to "一", finding zero space seems to be easier.

相较base64节省更多空间，更容易发现二进制文件的规律。

# Usage 使用说明

Clone this repo first.

首先克隆该仓库。

```bash
git clone https://github.com/fumiama/base16384.git
cd base16384
```

Then use `cmake` to vuild.

然后使用`cmake`进行构建。

```bash
mkdir build
cd build
cmake ..
make
```

Now you can use it to encode/decode a file.

现在可以使用命令对文件进行编码/解码。

```bash
Usage: -[e|d] <inputfile> <outputfile>
        -e encode
        -d decode
```