# Pizza-Delivery-POSIX-Threads
C language program runnable at Ubuntu Virtual Machine. The main goal is to deliver pizzas to multiple customers using POSIX-Threads. 

# Project for Operating Systems Course at Athens University of Econimics and Business (AUEB) 2024
A customer calls the pizzeria to make an order. If there is a telephone available, they begin their order, otherwise they wait until at least one telephone is available. During the order the customer can ask one of three pizza kinds (Margarita, Pepperoni, Special) from 1 to 5 in total. After they ask, they procceed to pay using credit card, with a small chance for the payment to fail which leads to the cancellation of the order. 
If the payment is successfull, the phone call ends and the telephone employee waits for the next phone call. At the same time, the bakers are waiting for an order. If their ovens are enough and they don't prepare another order, they put all the pizzas together in the oven and they start baking. After the pizzas are baked, the delivery guy packs and leaves the pizzeria to deliver the order. 

In this project, we are using pthreads to achieve true parallelism in our program. We are using both mutexes and condition variables for the phone calls, the baking, the packaging and the delivery of one order.
The program is runnable at Ubuntu Virtual Machine. 

To test the program we run these two commands:

- gcc -Wall -pthread p3190156-p3190254-pizza.c: This command debugs and checks for any kind of error or warning inside our code

- ./a.out 100 10: This command starts our program. The main method accepts two parameters.
  1) Number of customers
  2) Seed used for random number generation

Of course any other number is acceptable to test our code. If we wish to debug our program in much deeper depth, we can always modify our .h variables to see diffrence at both speed and results.

