#include <iostream>
#include <string>
#include <memory>
#include <vector>
#include <algorithm>
using namespace std;

struct BankAccount
{
    int balance = 0;
    int overdraft_limit = -500;
    
    void deposit(int amount)
    {
        balance += amount;
        cout << "deposited " << amount << ", balance now " <<
        balance << "\n";
    }
    
    bool withdraw(int amount)
    {
        if (balance - amount >= overdraft_limit)
        {
            balance -= amount;
            cout << "withdrew " << amount << ", balance now " <<
            balance << "\n";
            return true;
        }
        return false;
    }
    
    friend ostream &operator<<(ostream &os, const BankAccount &ba){
        cout << "Bank account balance is " << ba.balance << endl;
        return os;
    }
};

struct Command
{
    bool succeeded;
    virtual void call() = 0;
    virtual void undo() = 0;
};

// should really be BankAccountCommand
struct BankAccountCommand : Command
{
    BankAccount& account;
    enum Action { deposit, withdraw } action;
    int amount;
    
    BankAccountCommand(BankAccount& account, const Action action,
                       const int amount)
    : account(account),
    action(action), amount(amount)
    {
        succeeded = false;
    }
    
    void call() override
    {
        switch (action)
        {
            case deposit:
                account.deposit(amount);
                succeeded = true;
                break;
            case withdraw:
                succeeded = account.withdraw(amount);
                break;
        }
    }
    
    void undo() override
    {
        if (!succeeded) return;
        
        switch (action)
        {
            case withdraw:
                if (succeeded)
                    account.deposit(amount);
                break;
            case deposit:
                account.withdraw(amount);
                break;
        }
    }
};

// vector doesn't have virtual dtor, but who cares?
struct CompositeBankAccountCommand : vector<BankAccountCommand>, Command
{
    CompositeBankAccountCommand(const initializer_list<value_type>& _Ilist)
    : vector<BankAccountCommand>(_Ilist)
    {
    }
    
    void call() override
    {
        for (auto& cmd : *this)
            cmd.call();
    }
    
    void undo() override
    {
        for (auto it = rbegin(); it != rend(); ++it)
            it->undo();
    }
};

struct DependentCompositeCommand : CompositeBankAccountCommand
{
    explicit DependentCompositeCommand(
                                       const initializer_list<value_type>& _Ilist)
    : CompositeBankAccountCommand{ _Ilist } {}
    
    void call() override
    {
        bool ok = true;
        for (auto& cmd : *this)
        {
            if (ok)
            {
                cmd.call();
                ok = cmd.succeeded;
            }
            else
            {
                cmd.succeeded = false;
            }
        }
    }
};

struct MoneyTransferCommand : DependentCompositeCommand
{
    MoneyTransferCommand(BankAccount& from,
                         BankAccount& to, int amount):
    DependentCompositeCommand
    {
        BankAccountCommand{from, BankAccountCommand::withdraw, amount},
        BankAccountCommand{to, BankAccountCommand::deposit, amount}
    } {}
};


int main()
{
    cout << "1. money transfer for a small amount" << endl;
    BankAccount ba, ba2;
    ba.deposit(100);
    MoneyTransferCommand cmd{ba, ba2, 50};
    cmd.call();
    cout << ba << endl << ba2 << endl;
    cmd.undo();
    cout << ba << endl << ba2 << endl;
    
    cout << "2. money transfer for a large amount" << endl;
    MoneyTransferCommand cmd2{ba, ba2, 5000};
    cmd2.call();
    cout << ba << endl << ba2 << endl;
    cmd2.undo();
    cout << ba << endl << ba2 << endl;
    
    return 0;
}
