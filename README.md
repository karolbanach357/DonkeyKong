🎮 Cel projektu

Projekt to remake klasycznej gry „Donkey Kong” w wersji terminalowej / konsolowej lub w prostym środowisku graficznym (zależnie od implementacji). Celem było stworzenie gry w języku C++/C lub innym wybranym, z mechanikami: poziomy, skoki gracza i zbieranie punktów.

🛠️ Funkcje

Gracz porusza się po poziomie, unika przeszkód i wspina się.    
Liczenie punktów.  
Obsługa klawiatury (sterowanie ← →, skok, czasem wspinanie się).  
Wykorzystanie biblioteki graficznej 2D.  
Menu startowe, możliwość ponownego uruchomienia gry.  

⚙️ Uruchomienie / kompilacja

Sklonuj repozytorium:
`git clone https://github.com/karolbanach357/DonkeyKong.git`
`cd DonkeyKong`  
Skompiluj (przykład dla g++):  
`g++ main.cpp -o DonkeyKong -l[nazwa biblioteki]`
Uruchom grę:  
`./DonkeyKong`  
Sterowanie:  
← / → — ruch poziomy gracza  
Spacja — skok  
↑ / ↓ — wspinanie się   
Esc lub q — wyjście z gry
