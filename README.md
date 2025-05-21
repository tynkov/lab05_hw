## Laboratory work V

## Homework

### Задание

1. Создайте `CMakeList.txt` для библиотеки *banking*.
```cmake
cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(banking)

add_library(banking STATIC Transaction.cpp Account.cpp)
target_include_directories(banking PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
```

2. Создайте модульные тесты на классы `Transaction` и `Account`.
    * Используйте mock-объекты.
    * Покрытие кода должно составлять 100%.
```cpp
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <Account.h>
#include <Transaction.h>

class MockAccount: public Account{
public:
    MockAccount(int id, int balance): Account(id, balance){}
    MOCK_METHOD(int, GetBalance, (), (const, override)); 
    MOCK_METHOD(void, ChangeBalance, (int diff), (override)); 
    MOCK_METHOD(void, Lock, (), (override)); 
    MOCK_METHOD(void, Unlock, (), (override)); 
};

class MockTransaction: public Transaction{
public:
    MOCK_METHOD(void, SaveToDataBase, (Account& from, Account& to, int sum), (override));
};


TEST(Account, Banking){
    MockAccount test(0,0);
    
    ASSERT_EQ(test.GetBalance(), 0);
    
    ASSERT_THROW(test.ChangeBalance(100), std::runtime_error);
    
    test.Lock();
    
    ASSERT_NO_THROW(test.ChangeBalance(100));
    
    ASSERT_EQ(test.GetBalance(), 100);

    ASSERT_THROW(test.Lock(), std::runtime_error);

    test.Unlock();
    ASSERT_THROW(test.ChangeBalance(100), std::runtime_error);
}

TEST(Transaction, Banking){
    const size_t init_balance = 10000, init_fee = 100;
    
    MockAccount John(893, init_balance), Harry(1365, init_balance);
    MockTransaction payment_test;

    ASSERT_EQ(payment_test.fee(), 1);
    payment_test.set_fee(init_fee);
    ASSERT_EQ(payment_test.fee(), init_fee);

    ASSERT_THROW(payment_test.Make(John, John, 1000), std::logic_error);
    ASSERT_THROW(payment_test.Make(John, Harry, 0), std::logic_error);
    ASSERT_THROW(payment_test.Make(John, Harry, -50), std::invalid_argument);

    John.Lock();
    ASSERT_THROW(payment_test.Make(John, Harry, 1000), std::runtime_error);
    John.Unlock();

    ASSERT_EQ(payment_test.Make(John, Harry, 1000), true);
    ASSERT_EQ(John.GetBalance(), init_balance-1000-init_fee);
    ASSERT_EQ(Harry.GetBalance(), init_balance+1000);
}
```

3. Настройте сборочную процедуру на **TravisCI**.
```yml
name: gtest

on:
 push:
  branches: [main]
 pull_request:
  branches: [main]

jobs:
 build:

  runs-on: ubuntu-latest

  steps:
  - uses: actions/checkout@v3

  - name: Adding gtest
    run: git clone https://github.com/google/googletest.git third-party/gtest

  - name: Install lcov
    run: sudo apt-get install -y lcov

  - name: Config banking with tests
    run: cmake -H. -B ${{github.workspace}}/build -DBUILD_TESTS=ON

  - name: Build banking
    run: cmake --build ${{github.workspace}}/build

  - name: Run tests
    run: build/check
```

CMakeLists.txt
```cmake
cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(BUILD_TESTS "Build tests" OFF)

project(banking)

add_subdirectory(banking)

if(BUILD_TESTS)
  enable_testing()
  add_subdirectory(third-party/gtest)
  file(GLOB BANKING_TEST_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/tests/tests.cpp)
  add_executable(check ${BANKING_TEST_SOURCES})
  target_link_libraries(check banking gtest_main)
  add_test(NAME check COMMAND check)
endif()
```
4. Настройте [Coveralls.io](https://coveralls.io/).

## Links

- [C++ CI: Travis, CMake, GTest, Coveralls & Appveyor](http://david-grs.github.io/cpp-clang-travis-cmake-gtest-coveralls-appveyor/)
- [Boost.Tests](http://www.boost.org/doc/libs/1_63_0/libs/test/doc/html/)
- [Catch](https://github.com/catchorg/Catch2)

```
Copyright (c) 2015-2021 The ISC Authors
```
