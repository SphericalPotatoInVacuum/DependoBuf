# DependoBuf (DBuf)

- [DependoBuf (DBuf)](#dependobuf-dbuf)
  - [🌐 Overview](#-overview)
  - [⚙ Installation](#-installation)
  - [🛠 Building from Source](#-building-from-source)
    - [Prerequisites](#prerequisites)
    - [Cloning the repository](#cloning-the-repository)
    - [Pulling submodules](#pulling-submodules)
    - [Building the project](#building-the-project)
  - [📖 Usage](#-usage)
  - [🐋 Using Docker](#-using-docker)
  - [📜 License](#-license)


## 🌐 Overview

DependoBuf, or DBuf, is a data serialization format designed for structured data. It provides a flexible and precise approach to defining data structures with its unique feature: the ability to declare dependent types in its syntax.

Dependent types extend the expressiveness of type systems by allowing types to be predicated on values, leading to more robust and accurate data representations. DBuf's support for dependent types promotes the creation of more finely-tuned and efficient data models, making it a versatile tool in data-centric applications.

To work with DBuf, a DBuf "compiler" is used. It's currently focused on checking file validity by performing tasks that ensure the soundness of the defined schema. In the future, it will be extended to generate code for the defined schema in various languages.

## ⚙ Installation

At this time, precompiled builds of DependoBuf are not yet available. However, you can easily use DependoBuf by [building it from the source](#-building-from-source) or by [using the provided Docker image](#-using-docker).

## 🛠 Building from Source

### Prerequisites

Before building DependoBuf from source, please ensure the following tools and libraries are installed on your system:

- **CMake:** DependoBuf uses CMake as its build system. The minimum required version is 3.14.

- **Clang and libc++:** DependoBuf requires Clang version 16 or higher and libc++ version 16 or higher for building from source. You can install them from your package manager or from the [LLVM download page](https://releases.llvm.org/download.html).

- **Flex and Bison:** Flex (version 2.4 or higher) and Bison (version 3.8.2 or higher) are required to generate some parts of DependoBuf's source code. They can usually be installed from your package manager. For more details, see the [Flex manual](https://westes.github.io/flex/manual/) and the [Bison manual](https://www.gnu.org/software/bison/manual/).

- **Git:** DependoBuf's source code is hosted in a Git repository, so you'll need Git to clone the repository. It can be installed from your package manager or from the [Git website](https://git-scm.com/downloads).

Please note that it is your responsibility to comply with the licenses of all these dependencies.

> **Note**
> If you have multiple compilers installed, make sure to set Clang as the default compiler. This is because DependoBuf uses Clang-specific features and may not compile with other compilers. You can do that by setting the `CC` and `CXX` environment variables when running `build.sh`.

### Cloning the repository

Clone the DependoBuf repository to your local machine:

```
git clone --recurse-submodules https://github.com/SphericalPotatoInVacuum/DependoBuf.git
```

### Pulling submodules

Navigate to the project directory and initialize the submodules:

```
cd DependiBuf
```

### Building the project

To build the project you can use the `build.sh` script. It has the following arguments:
- `-h`: prints the help message
- `-t`: runs tests after building
- `-d`: builds in debug mode (default is release)
- `-i`: installs the dbuf tool after building
- `-j <n>`: builds with n threads (default is 1)

## 📖 Usage

After you've installed DependoBuf, you can use the `dbuf` command-line tool to validate your files. `dbuf` accepts a single argument: the path to the file you want to parse.

```bash
dbuf myfile.dbuf
```

In this command, `myfile.dbuf` is the file you want `dbuf` to parse.

`dbuf` parses the file and checks for any errors. If it encounters errors, it will print them to the standard output and exit with a non-zero status code. If the file is valid and no errors are found, `dbuf` will exit quietly with a status code of 0.

Here's an example output of `dbuf` when encountering errors:

```bash
$ dbuf myfile.dbuf
Got value of type "Float", but expected type is "Int" at 5.7-9
```

In this example, `dbuf` found a type mismatch errors in `myfile.dbuf` and printed an error message describing it. The numbers 5.7-9 are the line and columns numbers where the error was found.

## 🐋 Using Docker

If you'd prefer not to install or build anything on your local machine, you can use our Docker image. It comes with everything you need to use `DependoBuf` out of the box.

The image is available on Docker Hub under the name `sphericalpotatoinvacuum:dbuf`. You can use it to run `DependoBuf` on your files without any additional setup.

To run `DependoBuf` on a file using our Docker image, use the following command:

```bash
docker run -it -v $(pwd):$(pwd) -w $(pwd) sphericalpotatoinvacuum:dbuf myfile.dbuf
```

This command mounts your current directory (`$(pwd)`) into the Docker container and sets it as the working directory. This means that `DependoBuf` can access and interact with your files just like it would if it were running directly on your local machine.

For example, if you have a file named `myfile.dbuf` in your current directory, the above command will run `DependoBuf` on that file. Any output or errors will be displayed in your terminal as usual.

## 📜 License

This project is licensed under the terms of the GNU General Public License v3.0.

Under the GPL v3.0, you may freely use, modify, and distribute the code, provided that any modified versions or derivative works are also distributed under the GPL v3.0. For more details, please see the [LICENSE](LICENSE) file.
