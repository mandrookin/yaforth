del *.asm
echo off
for /r %%a in (*.frt) do (
  echo %%a
  yaforth -a %%a
)
           