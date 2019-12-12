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

// Variabelen die de pinnummers bijhouden...
int pinX = 16;
int pinY = 18;

static struct timer_list blink_timer;
static long data=0;

/*
 * The module commandline arguments ...
 */
static int 			 ioToggleSpeed	= 1;
static int 			 ioNummers[2] 	= { -1, -1 };
static int 			 arr_argc 		= 0;

module_param(ioToggleSpeed, int, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
MODULE_PARM_DESC(ioToggleSpeed, "An integer that describes how fast the IO toggles");
module_param_array(ioNummers, int, &arr_argc, 0000);
MODULE_PARM_DESC(ioNummers, "An array of integers met de IO-nummers");

/*
 * Clargmod init function
 */
static int clargmod_init(void)
{
	int i;

	printk(KERN_INFO "%s\n=============\n", __func__); // __func__ is de naam van de "enclosing function", hier clargmod_init.
	printk(KERN_INFO "ioToggleSpeed is an integer: %d\n", ioToggleSpeed);

	// Inhoud van de array met ioNummers tonen...
	for (i = 0; i < (sizeof ioNummers / sizeof (int)); i++)
	{
		printk(KERN_INFO "ioNummers[%d] = %d\n", i, ioNummers[i]);
	}

	// Controle op de inhoud van de array met ioNummers. Op basis van die inhoud worden de pinnen pinX en pinY juist gezet. 
	// Want het kan zijn dat de array leeg was, dat er 1 nummer in zat of 2 nummers in zaten.

	// Zit er niets op index 0 dan zal er ook niets zitten op index 1 dus krijgen pinX en pinY default waarden.
	if(ioNummers[0] == -1){
		pinX = 16;
	}else{
		pinX = ioNummers[0];
	}
	if(ioNummers[1] == -1){
		pinY = 18;
	}else{
		pinY = ioNummers[1];
	}
	printk(KERN_INFO "pinX : %d\n",pinX);
	printk(KERN_INFO "pinY : %d\n",pinY);	
	// Hoeveel argumenten zaten er in de array ioNummers...
	printk(KERN_INFO "got %d arguments for ioNummers.\n", arr_argc);

	return 0;
}
/*
 * Timer function called periodically
 */
static void blink_timer_func(struct timer_list* t)
{
	printk(KERN_INFO "%s\n", __func__);

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


	int ret1 = 0;
	int ret2 = 0;

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

	/* init timer, add timer function */
	//init_timer(&blink_timer);
	 timer_setup(&blink_timer, blink_timer_func, 0);

	// makes the LED toggle for "ioToggleSpeed" sec
	blink_timer.function = blink_timer_func;
	blink_timer.expires = jiffies + (ioToggleSpeed*HZ); 		 
	add_timer(&blink_timer);

	return ret1;
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

}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Stefan Wendler");
MODULE_DESCRIPTION("Basic kernel module using a timer and GPIOs to flash a LED.");

module_init(gpiomod_init);
module_exit(gpiomod_exit);
