'' 0 <= Dc <= 10 para generar entre 0 y 180 deg

var
  long th
  long tl

{pub main
  repeat
    pwm(20,4,16)
    ''pwm(20,70,16)
    ''pwm(20,1,16)
    ''waitcnt(clkfreq * 1 + cnt)}


pub pwm(tt,dc,pin) | timebase
  dira[pin]~~

  th := tt*DC/100
  tl := tt-th
  timebase := cnt
  waitcnt( clkfreq * tl / 1000 +timebase)
  !outa[pin]
  waitcnt( clkfreq * th / 1000 +timebase)
  !outa[pin]

