#include "malloc.h"

void *ft_memcpy(void *dst, const void *src, size_t n)
{
    unsigned char *d = dst;
    const unsigned char *s = src;
    
    while (n--)
        *d++ = *s++;
    return dst;
}

void ft_bzero(void *s, size_t n)
{
    unsigned char *ptr = s;
    
    while (n--)
        *ptr++ = 0;
}

void ft_putchar(char c)
{
    write(1, &c, 1);
}

void ft_putstr(const char *s)
{
    while (*s)
        ft_putchar(*s++);
}

void ft_putnbr(size_t n)
{
    if (n >= 10)
        ft_putnbr(n / 10);
    ft_putchar((n % 10) + '0');
}

void print_hex(size_t n)
{
    char hex[17] = "0123456789ABCDEF";
    char buffer[20];
    int i = 0;
    
    if (n == 0)
    {
        ft_putstr("0x0");
        return;
    }
    
    while (n > 0)
    {
        buffer[i++] = hex[n % 16];
        n /= 16;
    }
    
    ft_putstr("0x");
    while (i > 0)
        ft_putchar(buffer[--i]);
}