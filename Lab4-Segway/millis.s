40084834 <esp_timer_impl_get_counter_reg>:
40084834:       004136          entry   a1, 32
40084837:       f03c21          l32r    a2, 40080928 <_iram_text_start+0x524>
4008483a:       1a0c            movi.n  a10, 1
4008483c:       f03a91          l32r    a9, 40080924 <_iram_text_start+0x520>
4008483f:       0020c0          memw
40084842:       0938            l32i.n  a3, a9, 0
40084844:       0020c0          memw
40084847:       0288            l32i.n  a8, a2, 0
40084849:       f48d80          extui   a8, a8, 13, 16
4008484c:       88aa            add.n   a8, a8, a10
4008484e:       f03721          l32r    a2, 4008092c <_iram_text_start+0x528>
40084851:       0020c0          memw
40084854:       02a9            s32i.n  a10, a2, 0
40084856:       078876          loop    a8, 40084861 <esp_timer_impl_get_counter_reg+0x2d>
40084859:       0020c0          memw
4008485c:       0928            l32i.n  a2, a9, 0
4008485e:       ff9327          bne     a3, a2, 40084861 <esp_timer_impl_get_counter_reg+0x2d>
40084861:       f033a1          l32r    a10, 40080930 <_iram_text_start+0x52c>
40084864:       f03091          l32r    a9, 40080924 <_iram_text_start+0x520>
40084867:       0020c0          memw
4008486a:       0a38            l32i.n  a3, a10, 0
4008486c:       0020c0          memw
4008486f:       0988            l32i.n  a8, a9, 0
40084871:       049287          bne     a2, a8, 40084879 <esp_timer_impl_get_counter_reg+0x45>
40084874:       0020c0          memw
40084877:       f01d            retw.n
40084879:       082d            mov.n   a2, a8
4008487b:       fffa06          j       40084867 <esp_timer_impl_get_counter_reg+0x33>
        ...

40084880 <esp_timer_get_time>:
40084880:       004136          entry   a1, 32
40084883:       fffb25          call8   40084834 <esp_timer_impl_get_counter_reg>
40084886:       012b10          slli    a2, a11, 31
40084889:       41a1a0          srli    a10, a10, 1
4008488c:       2022a0          or      a2, a2, a10
4008488f:       4131b0          srli    a3, a11, 1
40084892:       f01d            retw.n