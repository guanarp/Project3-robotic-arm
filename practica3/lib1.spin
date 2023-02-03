pub frecuencia(f,pin,N)
    dira[pin]~~
    repeat N
      !outa[pin]
      waitcnt(cnt + clkfreq/(2*f))



    !outa[pin]
