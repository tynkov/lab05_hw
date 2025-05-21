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
    
    EXPECT_EQ(test.Account::GetBalance(), 0);
    
    EXPECT_THROW(test.Account::ChangeBalance(100), std::runtime_error);
    
    test.Lock();
    EXPECT_NO_THROW(test.Account::ChangeBalance(100));
    
    EXPECT_EQ(test.Account::GetBalance(), 100);

    EXPECT_THROW(test.Lock(), std::runtime_error);

    test.Unlock();
    EXPECT_THROW(test.Account::ChangeBalance(100), std::runtime_error);
}

TEST(Transaction, Banking){
    const size_t init_balance = 10000, init_fee = 100;
    
    MockAccount John(893, init_balance), Harry(1365, init_balance);
    MockTransaction payment_test;

    EXPECT_EQ(payment_test.fee(), 1);
    payment_test.set_fee(init_fee);
    EXPECT_EQ(payment_test.fee(), init_fee);

    EXPECT_THROW(payment_test.Make(John, John, 1000), std::logic_error);
    EXPECT_THROW(payment_test.Make(John, Harry, 0), std::logic_error);
    EXPECT_THROW(payment_test.Make(John, Harry, -50), std::invalid_argument);

    John.Lock();
    EXPECT_THROW(payment_test.Make(John, Harry, 1000), std::runtime_error);
    John.Unlock();

    EXPECT_EQ(payment_test.Make(John, Harry, 1000), true);
    EXPECT_EQ(John.GetBalance(), init_balance-1000-init_fee);
    EXPECT_EQ(Harry.GetBalance(), init_balance+1000);
}
