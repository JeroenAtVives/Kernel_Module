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

#define PIN16	16
#define PIN18	18

static struct timer_list blink_timer;
static long data=0;

/*
 * Timer function called periodically
 */
static void blink_timer_func(struct timer_list* t)
{
	printk(KERN_INFO "%s\n", __func__);

	gpio_set_value(PIN16, data);
	gpio_set_value(PIN18, data);
	data=!data; 
	
	/* schedule next execution */
	//blink_timer.data = !data;						// makes the LED toggle 
	blink_timer.expires = jiffies + (1*HZ); 		// 1 sec.
	add_timer(&blink_timer);
}

/*
 * Module init function
 */
static int __init gpiomod_init(void)
{
	int ret1 = 0;
	int ret2 = 0;

	printk(KERN_INFO "%s\n", __func__);

	// register, turn off 
	ret1 = gpio_request_one(PIN16, GPIOF_OUT_INIT_LOW, "pin16");
	ret2 = gpio_request_one(PIN18, GPIOF_OUT_INIT_LOW, "pin18");


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

	blink_timer.function = blink_timer_func;
	//blink_timer.data = 1L;							// initially turn LED on
	blink_timer.expires = jiffies + (1*HZ); 		// 1 sec.
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

	// turn LED off
	gpio_set_value(PIN16, 0); 
	gpio_set_value(PIN18, 0); 	
	
	// unregister GPIO 
	gpio_free(PIN16);
	gpio_free(PIN18);

}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Stefan Wendler");
MODULE_DESCRIPTION("Basic kernel module using a timer and GPIOs to flash a LED.");

module_init(gpiomod_init);
module_exit(gpiomod_exit);
