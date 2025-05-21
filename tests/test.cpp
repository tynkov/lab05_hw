#include <gtest/gtest.h>
#include "gmock/gmock.h"
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
