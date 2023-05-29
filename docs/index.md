# Introduction

DependoBuf is a novel structured data serialization format that incorporates
dependent types into the data model. It is designed to be a more expressive and
safe alternative by providing the ability to describe not only the structure of
the data, but the relationships between the data as well and enforcing those
relationships at both compile and runtime.

=== "Protocol Buffers"
    <div style="display: flex; width: 100%;">
    <div style="flex: 50%;">
    ```proto
    message TreeH {
        message TreeHLeaf {
            int32 value = 1;
            uint32 height = 2;
        }

        message TreeHNode {
            int32 value = 1;
            uint32 height = 2;
            TreeH left = 3;
            TreeH right = 4;
        }

        oneof treeh_type {
            TreeHLeaf leaf = 1;
            TreeHNode node = 2;
        }
    }
    ```
    </div>
    <div style="flex: 50%; margin: 16px;">

    :heavy_multiplication_x: Constraints not obvious

    :heavy_multiplication_x: No way to define constraints in schema

    :heavy_multiplication_x: No way to validate constraints soundness

    :heavy_multiplication_x: Need to write custom code to validate constraints
    </div>
    </div>
=== "DependoBuf"
    <div style="display: flex; width: 100%;">
    <div style="flex: 50%;">
    ```
    enum TreeH (height Unsigned) {
        0 => {
            TreeHLeaf {
                value Int
            }
        }
        * => {
            TreeHNode {
                value Int
                left TreeH (height - 1)
                right TreeH (height - 1)
            }
        }
    }
    ```
    </div>
    <div style="flex: 50%; margin: 16px;">

    :heavy_check_mark: Obvious constraints

    :heavy_check_mark: Constraint definition in schema

    :heavy_check_mark: Soundness validation at compile time

    :heavy_check_mark: Automatic validation of constraints at runtime
    </div>
    </div>

This documentation is meant to be a reference for the language and its features.
This documentation is not meant to be an explanation of the internal workings
of the compiler or the runtime. They are both works in progress, but you can
find the source code
[project's GitHub](https://github.com/SphericalPotatoInVacuum/DependoBuf/).

!!! warning
    This is a work in progress. The language and its features are subject to
    change. If you have any suggestions or feedback, please open an issue on
    [GitHub](https://github.com/SphericalPotatoInVacuum/DependoBuf/issues).
