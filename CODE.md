-----------------------------------------------------
Function Code               Meaning
-----------------------------------------------------
00                          Connected (Server)
01                          Disconnected (Server)
02                          Current Block
03                          Mined New Block
04                          New Transaction

-----------------------------------------------------
Buffer Payload
-----------------------------------------------------
Code    Payload                             Meaning
-----------------------------------------------------
00      0x000000000000000000000000000000    
01      0x000000000000000000000000000000    
02      0xa09165df41ab4bb2cde27001bac766    Current Block with Hash and Transaction
03      0x00000000000000000000000ab51c20    New Block with Nonce
04      0x00000000000000000000000ab51c20    New Transaction with from,to,amount,signature