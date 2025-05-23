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
