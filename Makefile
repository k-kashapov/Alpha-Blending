all: blend.exe

cpps = alpha.cpp
compilerFlags = -lraylib -lGL -lm -lpthread -ldl -lrt -lX11 -I include -mavx -mavx2

alpha.exe: 	$(cpps)
			g++ $(cpps) $(compilerFlags) -o alpha.exe -O1

alpha_o2.exe: 	$(cpps)
				g++ $(cpps) $(compilerFlags) -o alpha_o2.exe -O2

alpha_fast.exe: $(cpps)
				g++ $(cpps) $(compilerFlags) -o alpha_fast.exe -Ofast

alpha.s: 	$(cpps)
			g++ $(cpps) $(compilerFlags) -S -o alpha.s -masm=intel

blend.exe: 	ded.cpp
			g++ ded.cpp $(compilerFlags) -o blend.exe -Ofast

clean:
		rm ./alpha.exe
		rm ./alpha_o2.exe
		rm ./alpha_fast.exe
