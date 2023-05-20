/*
This file is part of DependoBuf project.

Copyright (C) 2023 Alexander Bogdanov, Alice Vernigor

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
*/
#include "glog/logging.h"

#include <gtest/gtest.h>

int main(int argc, char **argv) {
  FLAGS_logtostderr = true;
  google::InitGoogleLogging(argv[0]);

  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
