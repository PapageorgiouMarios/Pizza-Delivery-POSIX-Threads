#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include "p3190156-p3190254-pizza.h"

//---------Arguments for main-----------------
int N_CUST; //first argument: number of customers
unsigned int SEED; //second argument: seed used to generate numbers 
//--------------------------------------------

unsigned int sleep(unsigned int seconds); //sleep method for threads

int total_revenue = 0; // Total revenue, sum of all successful transactions

int available_telephones = N_TEL; // Number of all available telephones
int available_cooks = N_COOK; // Number of all available cooks
int available_ovens = N_OVEN; // Number of all available ovens
int available_deliverers = N_DELIVERER; // Number of all available deliverers

unsigned int total_pizzas_ordered = 0; // Common number of total pizzas ordered
unsigned int total_margaritas_ordered = 0; // Common number of total margarita pizzas ordered
unsigned int total_pepperonis_ordered = 0; // Common number of total pepperoni pizzas ordered
unsigned int total_specials_ordered = 0; // Common number of total special pizzas ordered

int number_of_orders = 0; // Number of orders in total
int successful_orders = 0; //Number of all completed orders
int failed_due_to_payment = 0; //number of all orders if payment failed

int max_service_time = 0; //Max waiting time printed at the end of all orders
float average_service_time_of_successful_orders = 0; // Average waiting time for all successful orders
double sum_service_time = 0; // Total waiting time for all customers

int max_cooling_time = 0; //Max cooling time printed at the end of all orders
float average_cooling_time_of_successful_orders = 0; // Average cooling time for all successful orders
double sum_cooling_time = 0; // Total cooling time for all customers

//--------------------------Mutexes----------------------------------------
pthread_mutex_t mutex_telephones; //used for telephones
pthread_mutex_t mutex_cooks; //used for cooks
pthread_mutex_t mutex_ovens; //used for ovens
pthread_mutex_t mutex_deliverers; //used for deliverers
pthread_mutex_t mutex_payment; //used for payment
pthread_mutex_t mutex_common_variables; //used to modify all threads' common variables
pthread_mutex_t mutex_print_screen; // used to print once in our screen
//-------------------------------------------------------------------------

//--------------------------Condition variables----------------------------
pthread_cond_t condition_telephones; //condition variable for the 2 telephones
pthread_cond_t condition_cooks; //condition variable for the 2 cooks
pthread_cond_t condition_ovens; //condition variable for the 10 ovens
pthread_cond_t condition_deliverers; //condition variable for the 10 deliverers
//-------------------------------------------------------------------------


void mutex_operation(pthread_mutex_t* mutex, int operation)
{
    int rc;

    if (operation == INITIALIZATION)
    {
        rc = pthread_mutex_init(mutex, NULL);

        if (rc != 0)
        {
            printf("\nError...Mutex Initialization failed\n");
            exit(-1);
        }
    }
    else if (operation == DESTRUCTION)
    {
        rc = pthread_mutex_destroy(mutex);

        if (rc != 0)
        {
            printf("\nError...Mutex Destruction failed\n");
            exit(-1);
        }
    }
    else if (operation == LOCK)
    {
        rc = pthread_mutex_lock(mutex);

        if (rc != 0)
        {
            printf("\nError...Mutex Lock failed\n");
            pthread_exit(&rc);
        }
    }
    else if (operation == UNLOCK)
    {
        rc = pthread_mutex_unlock(mutex);

        if (rc != 0)
        {
            printf("\nError...Mutex Unlock failed\n");
            pthread_exit(&rc);
        }
    }
}

void condition_operation(pthread_cond_t* condition, int operation)
{

    int rc;
    if (operation == INITIALIZATION)
    {
        rc = pthread_cond_init(condition, NULL);

        if (rc != 0)
        {
            printf("\nError..Condition Variable Initialization failed\n");
            exit(-1);
        }
    }
    else if (operation == DESTRUCTION)
    {
        rc = pthread_cond_destroy(condition);

        if (rc != 0)
        {
            printf("\nError..Condition Variable Destruction failed\n");
            exit(-1);
        }
    }

}

int generate_probabilty(float percentage, unsigned int seed)
{
    float generated_probability; // we take one float variable

    generated_probability = rand_r(&seed) % (100 + 1); // we generate a number with a max 100 for probability

    if (generated_probability < (1 - percentage))
    {
        return FAIL;
    }
    else
    {
        return SUCCESS;
    }

}

void telephone(CUSTOMER_ORDER* customer_order, int operation)
{
    if (operation == LOCK)
    {
        mutex_operation(&mutex_telephones, LOCK);

        while (1)
        {
            if (available_telephones > 0) // check if at least one telephone is available
            {
                available_telephones--;
                break;
            }
            else
            {
                pthread_cond_wait(&condition_telephones, &mutex_telephones); // we wait for a telephone to be available
            }
        }
        mutex_operation(&mutex_telephones, UNLOCK);

    }
    else if (operation == UNLOCK)
    {
        mutex_operation(&mutex_telephones, LOCK);
        available_telephones++;
        mutex_operation(&mutex_telephones, UNLOCK);
        pthread_cond_broadcast(&condition_telephones); // inform all threads that a telephone became available
    }

}

void cook(int operation)
{
    if (operation == LOCK)
    {
        mutex_operation(&mutex_cooks, LOCK);

        while (1)
        {
            if (available_cooks > 0) // check if at least one cook is available
            {
                available_cooks--;
                break;
            }
            else
            {
                pthread_cond_wait(&condition_cooks, &mutex_cooks); // we wait until one cook becomes available
            }
        }
        mutex_operation(&mutex_cooks, UNLOCK);
    }
    else if (operation == UNLOCK)
    {
        mutex_operation(&mutex_cooks, LOCK);
        available_cooks++;
        mutex_operation(&mutex_cooks, UNLOCK);
        pthread_cond_broadcast(&condition_cooks); // we inform our threads that a cook became available
    }
}

void bake(CUSTOMER_ORDER* customer_order, int operation) 
{
    if (operation == LOCK)
    {
        mutex_operation(&mutex_ovens, LOCK);

        while (1)
        {
            // For a specific order, we must check if the ovens are enough, since we bake them at the same time
            if (available_ovens > 0 && available_ovens >= customer_order->total_pizzas_ordered)
            {
                available_ovens -= customer_order->total_pizzas_ordered;
                break;
            }
            else
            {
                pthread_cond_wait(&condition_ovens, &mutex_ovens);
            }
        }
        mutex_operation(&mutex_ovens, UNLOCK);
    }
    else if (operation == UNLOCK)
    {
        mutex_operation(&mutex_ovens, LOCK);
        available_ovens+= customer_order->total_pizzas_ordered;
        mutex_operation(&mutex_ovens, UNLOCK);
        pthread_cond_broadcast(&condition_ovens);
    }
}

void delivery(int operation)
{
    if (operation == LOCK)
    {
        mutex_operation(&mutex_deliverers, LOCK);

        while (1)
        {
            if (available_deliverers > 0)
            {
                available_deliverers--;
                break;
            }
            else
            {
                pthread_cond_wait(&condition_deliverers, &mutex_deliverers);
            }
        }
        mutex_operation(&mutex_deliverers, UNLOCK);
    }
    else if (operation == UNLOCK)
    {
        mutex_operation(&mutex_deliverers, LOCK);
        available_deliverers++;
        mutex_operation(&mutex_deliverers, UNLOCK);
        pthread_cond_broadcast(&condition_deliverers);
    }
}

int ask_how_many_pizzas(unsigned int seed)
{
    int requested_pizzas;

    requested_pizzas = rand_r(&seed) % (N_ORDERHIGH + 1);

    if (requested_pizzas < N_ORDERLOW)
    {
        requested_pizzas = N_ORDERLOW;
    }

    return requested_pizzas;
}

int ask_what_kind_of_pizzas(unsigned int seed)
{
    float pizza_prob = rand_r(&seed) % (100 + 1);

    if (pizza_prob < (P_M * 100))
    {
        return MARGARITA;
    }
    else if (pizza_prob < (P_M + P_P) * 100)
    {
        return PEPPERONI;
    }
    else
    {
        return SPECIAL;
    }
}

void arguments_check(int argc, char* argv[])
{
    if (argc != 3) //3 because ./a.out 1, number of customers 2, seed 3
    {
        printf("\nError...Give 2 arguments.\n");
        exit(-1);
    }

    N_CUST = atoi(argv[1]);
    SEED = abs(atoi(argv[2]));

    if (N_CUST <= 0)
    {
        printf("\nError...The number of customers must be positive.\n");
        exit(-1);
    }
}

int main(int argc, char* argv[])
{
    arguments_check(argc, argv); //first we check if all arguments given are correct
    printf("Arguments used for pizzeria\n");
    printf("Number of customers: %d\n", N_CUST);
    printf("Number of seed: %d\n", SEED);

    //-----------------------INITIALIZE ALL MUTEXES-------------------------------
    mutex_operation(&mutex_telephones, INITIALIZATION);
    mutex_operation(&mutex_cooks, INITIALIZATION);
    mutex_operation(&mutex_ovens, INITIALIZATION);
    mutex_operation(&mutex_deliverers, INITIALIZATION);
    mutex_operation(&mutex_payment, INITIALIZATION);
    mutex_operation(&mutex_common_variables, INITIALIZATION);
    mutex_operation(&mutex_print_screen, INITIALIZATION);
    //-----------------------------------------------------------------------------

    //----------------------INITIALIZE ALL CONDITION VARIABLES----------------------
    condition_operation(&condition_telephones, INITIALIZATION);
    condition_operation(&condition_cooks, INITIALIZATION);
    condition_operation(&condition_ovens, INITIALIZATION);
    condition_operation(&condition_deliverers, INITIALIZATION);
    //------------------------------------------------------------------------------

    int customer_id[N_CUST]; //array of all customers' ids

    for (int id = 0; id < N_CUST; id++)
    {
        customer_id[id] = id + 1; //we want all customers have proper ids from 1 to number_of_customers
    }

    int rc;

    pthread_t running_threads[N_CUST];

    pthread_t* customers_threads;

    customers_threads = malloc(N_CUST * sizeof(pthread_t)); //we have to be careful for the memory we use

    if (customers_threads == NULL)
    {
        printf("\nNo memory../\n");
        return -1;
    }

    for (int i = 0; i < N_CUST; i++)
    {
        if (i == 0) //we want to create the first customer with id number 1
        {
            rc = pthread_create(&running_threads[i], NULL, order, &customer_id[i]);

            if (rc != 0)
            {
                printf("\nFirst thread creation failed!\n");
                exit(-1);
            }
        }
        else //the rest will follow in order with random showing-up time
        {
            int next_customer_generation_time = rand_r(&SEED) % (T_ORDERHIGH + 1 - T_ORDERLOW) + T_ORDERLOW;

            sleep(next_customer_generation_time);

            rc = pthread_create(&running_threads[i], NULL, order, &customer_id[i]);

            if (rc != 0)
            {
                printf("\nThread creation failed!\n");
                exit(-1);
            }

        }
    }

    void* stat; //stat used for joining method (usually null)

    for (int i = 0; i < N_CUST; i++)
    {
        rc = pthread_join(running_threads[i], &stat);

        if (rc != 0)
        {
            printf("\nThread join failed!\n");
            exit(-1);
        }
    }

    if(successful_orders > 0)
    {
        mutex_operation(&mutex_common_variables, LOCK);
        average_service_time_of_successful_orders = sum_service_time / successful_orders;
        average_cooling_time_of_successful_orders = sum_cooling_time / successful_orders;
        mutex_operation(&mutex_common_variables, UNLOCK);
    }

    // Print all statistics and calculated values

    mutex_operation(&mutex_print_screen, LOCK);
    printf("\n\nTotal orders: %d", number_of_orders);
    mutex_operation(&mutex_print_screen, UNLOCK);

    mutex_operation(&mutex_print_screen, LOCK);
    printf("\nTotal successful orders: %d", successful_orders);
    mutex_operation(&mutex_print_screen, UNLOCK);

    mutex_operation(&mutex_print_screen, LOCK);
    printf("\nTotal failed orders: %d", failed_due_to_payment);
    mutex_operation(&mutex_print_screen, UNLOCK);

    mutex_operation(&mutex_print_screen, LOCK);
    printf("\n\nTotal revenue: %d$", total_revenue);
    mutex_operation(&mutex_print_screen, UNLOCK);

    mutex_operation(&mutex_print_screen, LOCK);
    printf("\n\nTotal pizzas ordered: %d", total_pizzas_ordered);
    mutex_operation(&mutex_print_screen, UNLOCK);

    mutex_operation(&mutex_print_screen, LOCK);
    printf("\nTotal margaritas ordered: %d", total_margaritas_ordered);
    mutex_operation(&mutex_print_screen, UNLOCK);

    mutex_operation(&mutex_print_screen, LOCK);
    printf("\nTotal pepperonis ordered: %d", total_pepperonis_ordered);
    mutex_operation(&mutex_print_screen, UNLOCK);

    mutex_operation(&mutex_print_screen, LOCK);
    printf("\nTotal specials ordered: %d\n", total_specials_ordered);
    mutex_operation(&mutex_print_screen, UNLOCK);

    mutex_operation(&mutex_print_screen, LOCK);
    printf("\n\nAverage service time: %d minutes", (int) average_service_time_of_successful_orders);
    mutex_operation(&mutex_print_screen, UNLOCK);

    mutex_operation(&mutex_print_screen, LOCK);
    printf("\nMaximum service time: %d minutes", max_service_time);
    mutex_operation(&mutex_print_screen, UNLOCK);

    mutex_operation(&mutex_print_screen, LOCK);
    printf("\n\nAverage cooling time: %d minutes", (int) average_cooling_time_of_successful_orders);
    mutex_operation(&mutex_print_screen, UNLOCK);

    mutex_operation(&mutex_print_screen, LOCK);
    printf("\nMaximum cooling time: %d minutes\n", max_cooling_time);
    mutex_operation(&mutex_print_screen, UNLOCK);

    free(customers_threads); // free memory used for the threads (to avoid garbage staying)

    //-----------------------------DESTROY ALL MUTEXES--------------------------
    mutex_operation(&mutex_telephones, DESTRUCTION);
    mutex_operation(&mutex_cooks, DESTRUCTION);
    mutex_operation(&mutex_ovens, DESTRUCTION);
    mutex_operation(&mutex_deliverers, DESTRUCTION);
    mutex_operation(&mutex_payment, DESTRUCTION);
    mutex_operation(&mutex_common_variables, DESTRUCTION);
    mutex_operation(&mutex_print_screen, DESTRUCTION);
    //---------------------------------------------------------------------------

    //-----------------------------DESTROY ALL CONDITION VARIABLES------------------------
    condition_operation(&condition_telephones, DESTRUCTION);
    condition_operation(&condition_cooks, DESTRUCTION);
    condition_operation(&condition_ovens, DESTRUCTION);
    condition_operation(&condition_deliverers, DESTRUCTION);
    //------------------------------------------------------------------------------------

    return 0;

}

void* order(void* customer_id)
{
    int* cid = (int*)customer_id; // we get the customer's id
    CUSTOMER_ORDER c_order; // each customer has their own struct representing their order

    // We put default values in each customer's order (to avoid random default values for example -1253467)
    c_order.total_pizzas_ordered = 0;
    c_order.mergaritas_ordered = 0;
    c_order.pepperonis_ordered = 0;
    c_order.special_ordered = 0;
    c_order.cost = 0;

    unsigned int seed = SEED + *cid; // the seed for each customer must be unique

    struct timespec time_order_started; // Time the order started
    struct timespec time_order_baked; // Time the order finished baking
    struct timespec time_order_packed; // Time the order finished packing
    struct timespec time_order_delivered; // Time the order was delivered

    mutex_operation(&mutex_print_screen, LOCK);
    printf("\nCustomer <%d> is calling. \n", *cid); // when the customer shows up he/she calls the pizzeria
    mutex_operation(&mutex_print_screen, UNLOCK);

    double customer_service_time = 0; // keep track of customer's service time
    int preperation_time = 0;
    int delivered_time = 0;
    int cooling_time = 0;

    // The customer shows up and waits someone to pick up the phone
    clock_gettime(CLOCK_REALTIME, &time_order_started);

    telephone(&c_order, LOCK); // one of the two employees answers the phone

    // We have a new order
    mutex_operation(&mutex_common_variables, LOCK);
    number_of_orders = number_of_orders + 1; // +1 customer for service
    mutex_operation(&mutex_common_variables, UNLOCK);

    c_order.order_number = number_of_orders; // each customer has their own id

    int customer_pizza_number = ask_how_many_pizzas(seed); // how many pizzas the customer asked
    c_order.total_pizzas_ordered = customer_pizza_number; // save the number in struct

    mutex_operation(&mutex_common_variables, LOCK);
    total_pizzas_ordered += c_order.total_pizzas_ordered;
    mutex_operation(&mutex_common_variables, UNLOCK);

    for(int pizza = 0; pizza < customer_pizza_number; pizza++) // for each pizza the customer asked
    {
        // We must add something to the seed as well because if we put seed all by itself then
        // all pizzas will be the same

        int what_pizza = ask_what_kind_of_pizzas(seed + pizza); // MARGARITA/PEPPERONI/SPECIAL
        // Why + pizza?: Because every pizza the customer asks must be totally random.
        // If we put the same seed we might expect the same pizzas all the time

        if(what_pizza == MARGARITA) // in case one of the pizzas is a margarita
        {
            mutex_operation(&mutex_common_variables, LOCK);
            total_margaritas_ordered++;
            mutex_operation(&mutex_common_variables, UNLOCK);

            c_order.mergaritas_ordered++;
        }

        if (what_pizza == PEPPERONI) // in case one of the pizzas is a pepperoni
        {
            mutex_operation(&mutex_common_variables, LOCK);
            total_pepperonis_ordered++;
            mutex_operation(&mutex_common_variables, UNLOCK);

            c_order.pepperonis_ordered++;
        }

        if(what_pizza == SPECIAL) // in case one of the pizzas is a special
        {
            mutex_operation(&mutex_common_variables, LOCK);
            total_specials_ordered++;
            mutex_operation(&mutex_common_variables, UNLOCK);

            c_order.special_ordered++;
        }
    }

    mutex_operation(&mutex_print_screen, LOCK);
    printf("\nCustomer <%d> ordered %d pizzas", *cid, c_order.total_pizzas_ordered);
    mutex_operation(&mutex_print_screen, UNLOCK);

    mutex_operation(&mutex_print_screen, LOCK);
    printf("\nCustomer <%d> ordered %d margaritas", *cid, c_order.mergaritas_ordered);
    mutex_operation(&mutex_print_screen, UNLOCK);

    mutex_operation(&mutex_print_screen, LOCK);
    printf("\nCustomer <%d> ordered %d pepperonis", *cid, c_order.pepperonis_ordered);
    mutex_operation(&mutex_print_screen, UNLOCK);

    mutex_operation(&mutex_print_screen, LOCK);
    printf("\nCustomer <%d> ordered %d specials", *cid, c_order.special_ordered);
    mutex_operation(&mutex_print_screen, UNLOCK);

    int realize_payment_time = rand_r(&seed) % (T_PAYMENTHIGH + 1);

    if (realize_payment_time < T_PAYMENTLOW)
    {
        realize_payment_time = T_PAYMENTLOW;
    }

    sleep(realize_payment_time);

    int customer_bill = (c_order.mergaritas_ordered * C_M) + (c_order.pepperonis_ordered * C_P) +
        (c_order.special_ordered * C_S);

    c_order.cost = customer_bill;

    mutex_operation(&mutex_print_screen, LOCK);
    printf("\nCustomer <%d>:: Total cost for pizzas: %d$\n", *cid, c_order.cost);
    mutex_operation(&mutex_print_screen, UNLOCK);

    int payment_probability = generate_probabilty(P_CARDSUCCESS, seed);

    if (payment_probability == SUCCESS)
    {
        c_order.state = SUCCESS;

        mutex_operation(&mutex_print_screen, LOCK);
        printf("\nCustomer <%d>: Payment successful! Waiting for pizza\n", *cid);
        mutex_operation(&mutex_print_screen, UNLOCK);

        mutex_operation(&mutex_common_variables, LOCK);
        successful_orders++;
        mutex_operation(&mutex_common_variables, UNLOCK);

        mutex_operation(&mutex_payment, LOCK);
        total_revenue += c_order.cost;
        mutex_operation(&mutex_payment, UNLOCK);

        telephone(&c_order, UNLOCK); // The phone call ends. Time to make the pizzas
    }
    else
    {
        c_order.state = FAIL;

        mutex_operation(&mutex_print_screen, LOCK);
        printf("\nCustomer <%d>: Payment failed. Order is canceled\n", *cid);
        mutex_operation(&mutex_print_screen, UNLOCK);

        mutex_operation(&mutex_common_variables, LOCK);
        failed_due_to_payment++;
        mutex_operation(&mutex_common_variables, UNLOCK);

        telephone(&c_order, UNLOCK); // The phone call ends. Next customer...

        pthread_exit(NULL); // thread is no longer necessary for us to keep running the order
    }

    cook(LOCK); // we ask one of the cooks to bake an order
    sleep(T_PREP * c_order.total_pizzas_ordered); // the cook prepares the pizzas
    bake(&c_order, LOCK); // if the ovens are available the cook puts the order's pizzas at the same time
    sleep(T_BAKE); // we wait for the order's pizzas to be baked
    bake(&c_order, UNLOCK); // after the baking is complete the ovens are available
    cook(UNLOCK); // after the baking is complete the cook waits for another order

    clock_gettime(CLOCK_REALTIME, &time_order_baked); // get the moment the order is baked

    delivery(LOCK); // we get one deliverer to send the order
    sleep(T_PACK * c_order.total_pizzas_ordered); // we wait for the deliverer to package all pizzas

    clock_gettime(CLOCK_REALTIME, &time_order_packed); // get the moment the order is packaged
    preperation_time = time_order_packed.tv_sec - time_order_started.tv_sec;

    mutex_operation(&mutex_print_screen, LOCK);
    printf("\nCustomer <%d>: Order prepared in %d minutes\n", *cid, preperation_time);
    mutex_operation(&mutex_print_screen, UNLOCK);

    int delivery_time = rand_r(&seed) % (T_DELHIGH + 1);

    if (delivery_time < T_DELLOW)
    {
        delivery_time = T_DELLOW;
    }

    sleep(delivery_time);

    clock_gettime(CLOCK_REALTIME, &time_order_delivered); // get the moment the order reached the customer

    delivered_time = time_order_delivered.tv_sec - time_order_started.tv_sec;

    mutex_operation(&mutex_print_screen, LOCK);
    printf("\nCustomer <%d>: Order delivered in %d minutes\n", *cid, delivered_time);
    mutex_operation(&mutex_print_screen, UNLOCK);

    sleep(delivery_time); // don't forget the deliverer must return to the pizzeria
    delivery(UNLOCK);

    cooling_time = time_order_delivered.tv_sec - time_order_baked.tv_sec; // how long was the pizza cold

    customer_service_time = delivered_time; // what was the total service time for our customer

    if (customer_service_time > max_service_time) // always check for maximum value
    {
        mutex_operation(&mutex_common_variables, LOCK);
        max_service_time = customer_service_time;
        mutex_operation(&mutex_common_variables, UNLOCK);
    }

    if (cooling_time > max_cooling_time) // always check for maximum value
    {
        mutex_operation(&mutex_common_variables, LOCK);
        max_cooling_time = cooling_time;
        mutex_operation(&mutex_common_variables, UNLOCK);
    }

    mutex_operation(&mutex_common_variables, LOCK);
    sum_service_time += customer_service_time;
    mutex_operation(&mutex_common_variables, UNLOCK);

    mutex_operation(&mutex_common_variables, LOCK);
    sum_cooling_time += cooling_time;
    mutex_operation(&mutex_common_variables, UNLOCK);

    pthread_exit(NULL); // order is finished

}