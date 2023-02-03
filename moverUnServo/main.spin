obj
  servo : "servo"

pub main
  repeat
    servo.pwm(2,4,16)
    waitcnt(clkfreq + cnt)
    servo.pwm(2,8,16)
    waitcnt(clkfreq + cnt)
    ''pwm(20,70,16)
    ''pwm(20,1,16)
    ''waitcnt(clkfreq * 1 + cnt)