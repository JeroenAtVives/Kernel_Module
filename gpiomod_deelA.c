/*
 * Basic kernel module using a timer and GPIOs to flash a LED.
 *
 * Author:
 * 	Stefan Wendler (devnull@kaltpost.de)
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <linux/module.h>	
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/timer.h>
#include <linux/init.h>
#include <linux/gpio.h>

#include <linux/interrupt.h>

// Variabelen die de pinnummers bijhouden...
int pinX = 16;
int pinY = 18;

// Variabele voor input IO
int pinInput = -1;

// Variabele voor het aantal stijgende flanken...
int risingEdges = 0;

static struct timer_list blink_timer;
static long data=0;

/* Define GPIOs for BUTTONS */
static struct gpio buttons[] = {
		{ 17, GPIOF_IN, "BUTTON 1" }
};

/* Array voor het opslaan van IRQ numbers voor de button(s) */
static int button_irqs[] = { -1 };

/*
 * The module commandline arguments ...
 */
static int 			 ioToggleSpeed	= 1;
static int 			 ioNummers[2] 	= { -1, -1 };
static int			 input = -1;
static int 			 arr_argc 		= 0;

module_param(ioToggleSpeed, int, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
MODULE_PARM_DESC(ioToggleSpeed, "An integer that describes how fast the IO toggles");
module_param_array(ioNummers, int, &arr_argc, 0000);
MODULE_PARM_DESC(ioNummers, "An array of integers met de IO-nummers");
module_param(input, int,S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
MODULE_PARM_DESC(input, "Parameter die aangeeft welke IO als input wordt bekeken");
/*
 * Clargmod init function
 */
static int clargmod_init(void)
{
	int i;

	// Kernel messages...
	printk(KERN_INFO "%s\n=============\n", __func__); // __func__ is de naam van de "enclosing function", hier clargmod_init.
	printk(KERN_INFO "ioToggleSpeed is an integer: %d\n", ioToggleSpeed);
	printk(KERN_INFO "input is an integer: %d\n", input);

	// Inhoud van de array met ioNummers tonen...
	for (i = 0; i < (sizeof ioNummers / sizeof (int)); i++)
	{
		printk(KERN_INFO "ioNummers[%d] = %d\n", i, ioNummers[i]);
	}

	// Controle op de inhoud van de array met ioNummers. Op basis van die inhoud worden de pinnen pinX en pinY juist gezet. 
	// Want het kan zijn dat de array leeg was, dat er 1 nummer in zat of 2 nummers in zaten.


	if(ioNummers[0] == -1){
		pinX = 23;
	}else{
		pinX = ioNummers[0];
	}
	if(ioNummers[1] == -1){
		pinY = 24;
	}else{
		pinY = ioNummers[1];
	}

	// De input pin opvragen...
	if(input == -1){
		pinInput = 20;
	}else{
		pinInput = input;
	}

	// De struct correct invullen zodat we weten welke pin als button geïnterpreteerd moet worden...
	buttons[0].gpio = pinInput;
	buttons[0].flags = GPIOF_IN;
	buttons[0].label = "BUTTON 1";

	// Kernel messages...
	printk(KERN_INFO "pinX : %d\n",pinX);
	printk(KERN_INFO "pinY : %d\n",pinY);
	printk(KERN_INFO "input pin : %d\n",pinInput);	

	// Hoeveel argumenten zaten er in de array ioNummers...
	printk(KERN_INFO "got %d arguments for ioNummers.\n", arr_argc);

	return 0;
}

/*
 * The interrupt service routine called on button presses
 */
static irqreturn_t button_isr(int irq, void *data)
{
	if(irq == button_irqs[0]) {
		// Niewe flank
		risingEdges++;
		printk(KERN_INFO "Aantal stijgende flanken %d.\n",risingEdges);
		printk(KERN_INFO "flank detected");	
	}

	return IRQ_HANDLED;
}


/*
 * Timer function called periodically
 */
static void blink_timer_func(struct timer_list* t)
{
	printk(KERN_INFO "%s\n", __func__);

	// PinX en PinY setten...
	gpio_set_value(pinX, data);
	gpio_set_value(pinY, data);
	data=!data; 
	
	/* schedule next execution */
	blink_timer.expires = jiffies + (ioToggleSpeed*HZ); 		
	add_timer(&blink_timer);
}

/*
 * Module init function
 */
static int __init gpiomod_init(void)
{
	// Functie oproepen die zich bezig houdt met het verwerken van de parameters...
	clargmod_init();


	int retButtons = 0;
	int ret1 = 0;
	int ret2= 0;

	/* Register relais GPIO's (dus de pinnen beschreven in de array ioNummers)*/
	printk(KERN_INFO "%s\n", __func__);

	// register, turn off 
	ret1 = gpio_request_one(pinX, GPIOF_OUT_INIT_LOW, "pinX");
	ret2 = gpio_request_one(pinY, GPIOF_OUT_INIT_LOW, "pinY");


	if (ret1) {
		printk(KERN_ERR "Unable to request GPIOs: %d\n", ret1);
		return ret1;
	}

	if (ret2) {
	printk(KERN_ERR "Unable to request GPIOs: %d\n", ret2);
	return ret2;
	}


	/* register BUTTON gpios */
	retButtons = gpio_request_array(buttons, ARRAY_SIZE(buttons));

	if (retButtons) {
		printk(KERN_ERR "Unable to request GPIOs for BUTTONs: %d\n", retButtons);
	}

	printk(KERN_INFO "Current button1 value: %d\n", gpio_get_value(buttons[0].gpio));

	retButtons = gpio_to_irq(buttons[0].gpio);

	if(retButtons < 0) {
		printk(KERN_ERR "Unable to request IRQ: %d\n", retButtons);
	}

	button_irqs[0] = retButtons;

	printk(KERN_INFO "Successfully requested BUTTON1 IRQ # %d\n", button_irqs[0]);

	retButtons = request_irq(button_irqs[0], button_isr, IRQF_TRIGGER_RISING /* | IRQF_DISABLED */, "gpiomod#button1", NULL);

	if(retButtons) {
		printk(KERN_ERR "Unable to request IRQ: %d\n", retButtons);
	}




	/* init timer, add timer function */
	//init_timer(&blink_timer);
	 timer_setup(&blink_timer, blink_timer_func, 0);

	// makes the LED toggle depending on "ioToggleSpeed"
	blink_timer.function = blink_timer_func;
	blink_timer.expires = jiffies + (ioToggleSpeed/10*HZ); 		 
	add_timer(&blink_timer);

	return retButtons;
}

/*
 * Module exit function
 */
static void __exit gpiomod_exit(void)
{
	printk(KERN_INFO "%s\n", __func__);

	// deactivate timer if running
	del_timer_sync(&blink_timer);

	// Zet de IOpinnen terug laag...
	gpio_set_value(pinX, 0); 
	gpio_set_value(pinY, 0); 	
	
	// unregister GPIO 
	gpio_free(pinX);
	gpio_free(pinY);
	gpio_free_array(buttons, ARRAY_SIZE(buttons));


	// free irqs
	free_irq(button_irqs[0], NULL);
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Stefan Wendler");
MODULE_DESCRIPTION("Basic kernel module using a timer and GPIOs to flash a LED.");

module_init(gpiomod_init);
module_exit(gpiomod_exit);
