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
#include <Account.h>
#include <Transaction.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

using ::testing::_;
using ::testing::Return;
using ::testing::InSequence;
using ::testing::Throw;

class MockAccount : public Account {
public:
	MockAccount(int id, int balance) : Account(id, balance) {}

	MOCK_METHOD(int, GetBalance, (), (const, override));
	MOCK_METHOD(void, ChangeBalance, (int), (override));
	MOCK_METHOD(void, Lock, (), (override));
	MOCK_METHOD(void, Unlock, (), (override));
};

class MockTransaction : public Transaction {
public:
	MOCK_METHOD(void, SaveToDataBase, (Account&, Account&, int), (override));
};

TEST(Account, ChangeBalanceWOLock) {
	Account account(1, 100);
	EXPECT_THROW(account.ChangeBalance(10), std::runtime_error);
}

TEST(Account, LockUnlock) {
	Account account(1, 100);

	account.Lock();
	EXPECT_THROW(account.Lock(), std::runtime_error);
	account.Unlock();
	EXPECT_NO_THROW(account.Lock());
}

TEST(Account, GetBalanceValue) {
	Account account(1, 150);
	account.Lock();
	account.ChangeBalance(-50);
	account.Unlock();
	EXPECT_EQ(account.GetBalance(), 100);
}

TEST(Transaction, SameAccount) {
	MockAccount account(1, 1000);
	MockTransaction tx;
	EXPECT_THROW(tx.Make(account, account, 100), std::logic_error);
}

TEST(Transaction, NegativeSum) {
	MockAccount from(1, 1000);
	MockAccount to(2, 1000);
	MockTransaction tx;
	EXPECT_THROW(tx.Make(from, to, -100), std::invalid_argument);
}

TEST(Transaction, SmallSum) {
	MockAccount from(1, 1000);
	MockAccount to(2, 1000);
	MockTransaction tx;
	EXPECT_THROW(tx.Make(from, to, 50), std::logic_error);
}

TEST(Transaction, HighFee) {
	MockAccount from(1, 1000);
	MockAccount to(2, 1000);
	MockTransaction tx;
	tx.set_fee(100);

	EXPECT_FALSE(tx.Make(from, to, 100));
}

TEST(Transaction, Successful) {
	MockAccount from(1, 1000);
	MockAccount to(2, 1000);
	MockTransaction tx;
	tx.set_fee(10);

	{
		EXPECT_CALL(from, Lock());
		EXPECT_CALL(to, Lock());

		EXPECT_CALL(to, ChangeBalance(200));
		EXPECT_CALL(to, GetBalance()).WillOnce(Return(1200));
		EXPECT_CALL(to, ChangeBalance(-210));
		EXPECT_CALL(from, GetBalance()).WillRepeatedly(Return(1000));
    
		EXPECT_CALL(tx, SaveToDataBase(_, _, 200));
	}

	EXPECT_CALL(from, Unlock());
	EXPECT_CALL(to, Unlock());

	EXPECT_TRUE(tx.Make(from, to, 200));
}

TEST(Transaction, NotEnoughFunds) {
	MockAccount from(1, 1000);
	MockAccount to(2, 1000);
	MockTransaction tx;
	tx.set_fee(10);

	{
		EXPECT_CALL(from, Lock());
		EXPECT_CALL(to, Lock());

		EXPECT_CALL(to, ChangeBalance(100));
		EXPECT_CALL(to, GetBalance()).WillOnce(Return(100));
		EXPECT_CALL(to, ChangeBalance(-100));

		EXPECT_CALL(tx, SaveToDataBase(_, _, 100));
	}

	EXPECT_CALL(from, Unlock());
	EXPECT_CALL(to, Unlock());

	EXPECT_FALSE(tx.Make(from, to, 100));
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

project(lab05)

add_subdirectory(banking)

if(BUILD_TESTS)
  enable_testing()
  add_subdirectory(third-party/gtest)
  file(GLOB BANKING_TEST_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/tests/test.cpp)
  add_executable(check ${BANKING_TEST_SOURCES})
  target_link_libraries(check banking gtest_main gmock_main)
  add_test(NAME check COMMAND check)
endif()
```
4. Настройте [Coveralls.io](https://coveralls.io/).
```yml
name: CI with gtest and lcov

on:
 push:
  branches: [main]
 pull_request:
  branches: [main]

jobs:
 build:

  runs-on: ubuntu-latest

  steps:
  - uses: actions/checkout@v4

  - name: Adding gtest
    run: git clone https://github.com/google/googletest.git third-party/gtest

  - name: Install cpp-coveralls
    run: pip install cpp-coveralls

  - name: Config banking with tests
    run: cmake -H. -B${{github.workspace}}/build -DBUILD_TESTS=ON -DCODE_COVERAGE=ON

  - name: Build banking
    run: cmake --build ${{github.workspace}}/build

  - name: Run tests
    run: build/check --enable-gcov

  - name: Measure coverage
    env:
      COVERALLS_REPO_TOKEN: ${{ secrets.COVERALLS_REPO_TOKEN }}
    run: coveralls -r build
```
CMakeLists.txt
```cmake
cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(BUILD_TESTS "Build tests" OFF)
option(CODE_COVERAGE "Enable coverage reporting" OFF)

project(lab05)

add_subdirectory(banking)

if(BUILD_TESTS)
  enable_testing()
  add_subdirectory(third-party/gtest)
  file(GLOB BANKING_TEST_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/tests/test.cpp)
  add_executable(check ${BANKING_TEST_SOURCES})
  target_link_libraries(check banking gtest_main gmock_main)
  add_test(NAME check COMMAND check)
endif()

if(CODE_COVERAGE AND CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
  target_compile_options(check PRIVATE --coverage)
  target_link_options(check PRIVATE --coverage)
endif()
```
## Links

- [C++ CI: Travis, CMake, GTest, Coveralls & Appveyor](http://david-grs.github.io/cpp-clang-travis-cmake-gtest-coveralls-appveyor/)
- [Boost.Tests](http://www.boost.org/doc/libs/1_63_0/libs/test/doc/html/)
- [Catch](https://github.com/catchorg/Catch2)

```
Copyright (c) 2015-2021 The ISC Authors
```
