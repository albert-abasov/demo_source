package com.example.account;

/**
 * Represents a simple user account in the system.
 */
public class UserAccount {

    // Unique identifier for the user
    private int id;

    // Username used for login
    private String username;

    // Account balance in USD
    private double balance;

    /**
     * Creates a new UserAccount.
     *
     * @param id Unique user ID
     * @param username Username for login
     */
    public UserAccount(int id, String username) {
        this.id = id;
        this.username = username;
        this.balance = 0.0;
    }

    /**
     * Deposits money into the account.
     *
     * @param amount Amount to deposit
     */
    public void deposit(double amount) {
        if (amount > 0) {
            balance += amount;
        }
    }

    /**
     * Withdraws money from the account.
     *
     * @param amount Amount to withdraw
     * @return true if withdrawal was successful
     */
    public boolean withdraw(double amount) {
        if (amount > 0 && balance >= amount) {
            balance -= amount;
            return true;
        }
        return false;
    }

    /**
     * Gets current balance.
     *
     * @return account balance
     */
    public double getBalance() {
        return balance;
    }

    /**
     * Gets username.
     *
     * @return username
     */
    public String getUsername() {
        return username;
    }

    /**
     * Static utility method to create a default guest account.
     *
     * @return guest UserAccount
     */
    public static UserAccount createGuestAccount() {
        return new UserAccount(0, "guest");
    }

    /**
     * Overloaded deposit method.
     */
    public void deposit(int amount) {
        deposit((double) amount);
    }
}
