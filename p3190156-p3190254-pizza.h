#pragma once
#include <pthread.h>

//---------------------------------------------PIZZERIA--------------------------------------------------------------
#define N_TEL 2 // Telephones: The employee answers the phone and takes the customer's order.Also, he/she handles the payment 
#define N_COOK 2 // Cooks: After the customer's order is completed successfully, the cook uses avens to bake the pizzas
#define N_OVEN 10 // Ovens: The cooks need ovens to bake. When the proper number of ovens is available the cook puts the pizzas in
#define N_DELIVERER 10 // Deliverers: After the pizzas are baked one deliverer takes the pizzas, package them and delivers them. After that he/she returns to the pizzeria
//------------------------------------------------------------------------------------------------------------------

//------------------------------------NEXT THREAD CREATION TIME------------------------------
#define T_ORDERLOW 1 // Minimum number of seconds until the next customer (thread) appears
#define T_ORDERHIGH 5 // Maximum number of seconds until the next customer (thread) appears
//-------------------------------------------------------------------------------------------

//--------------------------------HOW MANY PIZZAS THE CUSTOMER ASKS--------------------------
#define N_ORDERLOW 1 // ,Minimum number of pizzas ordered
#define N_ORDERHIGH 5 // Maximum number of pizzas ordered
//-------------------------------------------------------------------------------------------

//--------------------------------WHAT PIZZAS THE CUSTOMER WANTS-----------------------------
#define P_M 0.35 // probability of margarita pizza (35%)
#define P_P 0.25 // probability of pepperoni pizza (25%)
#define P_S 0.40 // probability of special pizza (40%)
//-------------------------------------------------------------------------------------------

//--------------------------HOW LONG THE PHONE GUY NEED TO DO THE PAYMENT--------------------
#define T_PAYMENTLOW 1 // minimum duration of customer's charging
#define T_PAYMENTHIGH 3 // maximum duration of customer's charging
//-------------------------------------------------------------------------------------------

//-------------------------WHAT IS THE SUCCESSFUL PAYMENT RATE-------------------------------
#define P_CARDSUCCESS 0.95 // probability of payment success (95%)
#define P_CARDFAILURE 0.05 // probability of payment failure (5%)
//-------------------------------------------------------------------------------------------

//-------------------------HOW MUCH EACH PIZZA KIND COST-------------------------------------
#define C_M 10 // cost of margarita pizza in euros (10€)
#define C_P 11 // cost of pepperoni pizza in euros (11€)
#define C_S 12 // cost of special pizza in euros (12€)
//-------------------------------------------------------------------------------------------

//-----------------------------PIZZAS' PREPERATION-------------------------------------------
#define T_PREP 1 // STEP 1: Prepare the pizzas 
#define T_BAKE 10 // STEP 2: Bake the pizzas simultaneously in the ovens
#define T_PACK 1 // STEP 3: Package all baked pizzas

//----------------STEP 4: Deliver the pizzas------------------
#define T_DELLOW 5 // Minimum delivery time
#define T_DELHIGH 15 // Maximum delivery time
//------------------------------------------------------------
//-------------------------------------------------------------------------------------------

//--------------------------------REPRESENT PIZZA'S KIND-------------------------------------
#define MARGARITA 111 // Variable to represent margarita
#define PEPPERONI 222 // Variable to represent pepperoni
#define SPECIAL 333 // Variable to represent special
//-------------------------------------------------------------------------------------------

//---------------MUTEX OPERATIONS---------------
#define INITIALIZATION 0 
#define DESTRUCTION 1
#define LOCK 2 
#define UNLOCK 3
//----------------------------------------------

//---------------------------WAS THE ORDER SUCCESSFUL OR FAILED------------------------------
#define SUCCESS 0 //variable to represent success
#define FAIL 1 //variable to represent failure
//-------------------------------------------------------------------------------------------

//----------------------CUSTOMER ORDER-------------------------
typedef struct customer_order 
{
    int order_number; //Number of order (id)
    int total_pizzas_ordered; // Number of pizzas ordered (include all kinds)
    int mergaritas_ordered; // How many of them were margarita
    int pepperonis_ordered; // How many of them were pepperoni
    int special_ordered; // How many of them were special
    int cost; // Total price the customer pays
    int state; // State to represent the case the customer's order is accepted or rejected due to unsuccessful payment
}CUSTOMER_ORDER;
//--------------------------------------------------------------

/*
* mutex_operation: We try to organize our mutex operations so that we avoid
* to repeat the same lines of code for the mutexes and make the .c file's understanding harder
*/
void mutex_operation(pthread_mutex_t* mutex, int operation);

/*
* condition_operation: We try to organize our condition variables operations so that we avoid
* to repeat the same lines of code for the mutexes and make the .c file's understanding harder
*/
void condition_operation(pthread_cond_t* cond, int operation);

/*
* generate_probabilty: Used to calculate the payment's percentage using the customer's seed
*/
int generate_probabilty(float percentage, unsigned int seed); //generator for random probabilities

/*
* telephone: There we handle the pizzeria's telephones. When the employee answers the phone the customer gets
* his/her order id. Inside the code we handle both mutexes and condition variables for phones
*/
void telephone(CUSTOMER_ORDER* order, int operation);

/*
* cook: There we handle the pizzeria's cooks. When one order is prepared, the cook must take the pizzas and
* bake them in the ovens. Of course, the cook must wait for the proper number of ovens to be available.
* Inside the code we handle both mutexes and condition variables for cooks
*/
void cook(int operation);

/*
* bake: There we handle the pizzeria's ovens. Like we mentioned before, the cook must wait for the available
* number of ovens to be available so that he/she can put the order's pizzas at the same time. Compared to 
* the previous methods we check the crowd of pizzas asked by the customer. 
* Inside the code we handle both mutexes and condition variables for ovens
*/
void bake(CUSTOMER_ORDER *customer_order, int operation);

/*
* delivery: There we handle the pizzeria's delivery guys. When one order is ready to go, one deliverer must
* package all pizzas and ride them to the customer. When they reach the customer the order is complete.
* But we must take notice of the deliverer to come back to the pizzeria
*/
void delivery(int operation); //method to call mutex operations for all deliverers

/*
* ask_how_many_pizzas: A simple number generator from N_ORDERLOW to N_ORDERHIGH (1 to 5 pizzas)
*/
int ask_how_many_pizzas(unsigned int seed);

/*
* ask_what_kind_of_pizzas: After we get how many pizzas a customer wants, for each pizza
* we use a number generator to determine each pizza's kind
*/
int ask_what_kind_of_pizzas(unsigned int seed);

/*
* order: There is the whole order process for one customer:
* Step by step:
* - A customer calls the pizzeria "I want to order some pizza"
* 
* - One of the two phone guys answer the phone "How may I help you?". 
    If both employees are already talking, the rest of the customers wait

  - The customer gives his/her order for example "I want two margaritas and one special pizza"

  - The phone guy need some time to charge the customer's credit card (still on the phone)
    "Please give me some minutes to make the payment"

  - Scenario 1: The charge has been accepted and the order is saved "Ok, it's 32 euros. Thanks for odering"
  - Scenario 2: The charge has been rejected and the order is cancelled "Sorry, but your card has a problem"

  - The phone guy hangs up and is ready to accept another phone call

  - When one order is saved the cook is informed "We need 2 margaritas and 1 special pizza"
  - The cook waits the ovens to be available "I need 3 ovens to bake the pizzas"
  - When the pizzas are put in the ovens, we wait for the order to be baked "Ok they are ready take them"

  - One deliverer packages all pizzas "Ok let me take 3 boxes and package the order"
  - After packaging, the deliverer gets on the bike and goes to the address "Aueb, 28th October street"
  - The bike ride takes 5 to 15 minutes "Ok this is Aueb"
  - The deliverer gives the pizzas to the customer "Here is your order"
  - The customer's order is completed "Thank you very much. Time to eat!"
  - The deliverer comes back to the pizzeria "Time to return"
* 
*/
void* order(void* customer_id); //method for the total transaction(used by all customer threads)

/*
* When we run our program we use a .sh file which sets the arguments
* Of course we always need to check what arguments are we given
*/
void arguments_check(int argc, char* argv[]); //method to check all arguments for the a.out and the .sh file