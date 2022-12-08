# R5-Internal
This was an internal trainer for the game Apex Legends.<br>
It has been maintained from the start of Season 9 to the end of Season 12.
<br><br>
Confidential parts have been stripped, however features and some utility are retained in this source code.<br/>
You should only use it for referencing, as the code is suboptimally written.
# Features
 - Aimbot (Regular/Silent/ghetto PSilent) 
 - ESP
 - Highlight
 - Bunnyhop
 - Freecam
 - Thirdperson
 - Air stuck
 - Fake Lag
 - Heirloom Changer [>reference](https://www.unknowncheats.me/forum/apex-legends/488411-heirloom-animation-fix.html)
 - Infinite Charge Rifle [>reference](https://www.unknowncheats.me/forum/apex-legends/489948-airstuck-chargerifle-hack-infinite-chargerifle.html) (meme method, only works on high ping)
 # History
 Although the anti-cheat evasion method was nothing spectacular, this trainer has managed to stay under the radar for several seasons, used by only two people.
 In earlier seasons where the server-side mechanisms were a bit lower, you could drop 4k 20s all day and keep your account for several seasons. This changed somewhere around
 Season 11, whereas now you will get auto-banned, if you drop high kill games and get reported alot. More in depth factors about this new system are now known to me,
 i am only talking about my experience. So my evidence is by no doubt anecdotal.
 # Evasion Method
 Only one evasion method was used through the whole lifetime and it was done by hooking EasyAntiCheat's kernel driver routine where it maps their internal game module.<br>
 Some shenanigans were done to make the anti-cheat believe R5I was a part of such module. This can easily be detected however, so consider it patched today.<br>
 # Conclusion
 All in all this wasn't a very innovative evasion method. However combined with disarming stack walking functions in user-mode and kernel, you had a cheap bypass ready.<br/>
 Although being far from fully invisible, it was seemingly enough to last a decent amount of time without getting banned.<br>
 
