(Treść zadania była napisana po polsku, więc README do niego też napiszę po polsku)

Jak uruchomić projekt
=====================
Tak samo, jak w zadaniu, które mi wysłaliście:
```
Jak odpalić multiplayer: 
    ○ W Play Settingsach ustaw: Number of Players - 2, Play Mode - Standalone 
    ○ Odpalaj z levelu Lvl_Menu 
    ○ Alternatywnie możesz też ustawić Net Mode “Play as Listen Server” i odpalać grę z Lvl_Game na Selected Viewport 
```

**UWAGA**: Nie zaimplementowałem żadnych animacji do strzelania, więc w przypadku AssaultRifle może wyglądać jakby nic nie robiło - dodałem tam drobny delay przed pierwszą serią, więc trzeba trochę dłużej przytrzymać.

Architektura, decyzje techniczne i uwagi
===============================================

Inventory
---------
Inventory (UInventoryComponent) nie jest replikowane, a zamiast tego lokalny klient, którego inventory to jest oraz serwer mają własne kopie, które są niezależnie updatowane przez inne, replikowane akcje gameplayowe (np. pickupy czy strzelanie).

To może w pewnych sytuacjach spowodować rozjazd między serwerem a graczem (np. jeśli gracz będzie chciał strzelić z pistoletu, odejmie sobie sztukę amunicji, a serwer powie mu, że nie może teraz strzelić z jakiegoś powodu). Nie skupiałem się na tym na potrzeby tego zadania, ale w takich sytuacjach wymagałoby to albo cofnięcia akcji usunięcia amunicji z inventory, jeśli dostaniemy taką odpowiedź od serwera, albo zsynchronizowania inventory na lokalnym kliencie ze stanem na serwerze.

**Przedmioty**

Początkowo rozważałem nad stworzeniem definicji przedmiotu jako DataAsset lub jakiś customowy asset i tworzenie jego instancji i zapisywanie ich w folderach, ale później stwierdziłem, że chciałbym, żeby przedmioty różnego typu miały różne propertiesy, niektóre ich więcej, niektóre mniej, a jedne potencjalnie  dziedziczące po innych.
Dlatego zdecydowałem się do użycia jako definicji przedmiotów, data-only blueprintów. Ich instancje nie są nigdy tworzone w grze (poza DefaultValueObjectem oczywiście, z którego możemy w grze propertiesy takiego przedmiotu sczytywać).

Dałoby się ten sam efekt osiągnąć przy pomocy customowych Assetów, ale blueprinty są na to szybszym w implementacji sposobem (nawet jeśli niekoniecznie lepszym) i pozwalają łatwiej na dodawanie nowych propertiesów do przedmiotów w blueprintach, gdyby była taka potrzeba w ramach prototypu.

Podnoszenie amunicji
--------------------

**Pickup**

Stworzyłem bazową klasę dla wszystkich pickupów: APickupBase, która implementuje podstawową replikację i logikę związaną z podnoszeniem rzeczy i walidacją tego z serwerem.

Dla ułatwienia ma specjalizację APickupInventoryItem, która od razu implementuje logikę dla podnoszenia wybranego przedmiotu. Można je bezpośrednio położyć na level i ustawić na instancji parametry (co za przedmiot ma być podniesiony i w jakiej ilości; a także odziedziczone z bazowego pickupu parametry związane z wyglądem), lub podziedziczyć (nawet data-only blueprintem, żeby tylko ustawić parametry).

Chciałem, by pickup amunicji podnosił typ i liczbę amunicji zależną od broni, którą aktualnie mamy w ręce, a równocześnie chciałem z jednego miejsca w edytorze ustawiać materiały, rozmiary kolizji itd. dla wszytkich pickupów, dlatego w blueprintach zaimplementowane jest (data only) BP_PickupBase, po którym dziedziczy BP_AmmoPickup implementująca tę odrobinę logiki w blueprintach.

Replikacja napisana jest tak, by lokalny gracz odczuwał jak najmniejsze opóźnienie, nie zaimplementowałem jednak co się powinno dziać u lokalnego gracza, jeśli serwer stwierdzi, że nie mógł jednak podnieść tego pickupu. Nie cofam obecnie w żaden sposób stanu u lokalnego gracza i może to powodować bugi kiedy np. w wyniku laga dwóch graczy będzie twierdziło, że podniosło ten sam pickup (a serwer uzna to tylko jednemu). To coś, co warto by doimplmentować.

**Spawner**

Mamy klasę bazową dla wszystkich spawnerów APickupSpawnerBase, która zajmuje się replikacją - jako że pickupy są replikowane same z siebie, nie replikuję samego faktu spawnowania, ale replikuję dwa floaty potrzebne do tego, by na wszystkich maszynach pokazywało tak samo, ile zostało czasu do spawnu.

W blueprintach dziedziczy po niej BP_PickupSpawnerBase, która tylko dodaje de facto progress bar na shaderze i go updatuje.

Instancje tego mogłyby już być kładzione na levelu, ale dodałem jeszcze BP_AmmoSpawner, który tylko ustawia zmienne, żeby nie musieć ich wszędzie zmieniać na levelu przy tweakowaniu.


Ekwipunek i PlayerAbility
-------------------------

UEquipmentComponent wskazuje na to, jaką broń aktualnie gracz na wybraną i odpowiada za ustawianie rzeczy z nią związanych (PlayerAbility) oraz replikację tego stanu do innych graczy.

Różne bronie mogą być używane na różne sposoby - w tym zadaniu było strzelanie w sposób automatyczny lub półautomatyczny, ale gdyby były jakieś chargowane strzały, czy lobowane rzuty czy ataki w zwarciu, każde z nich mogłoby mieć osobną logikę, więc postanowiłem zamknąć tę logikę w klasach, które nazwałem PlayerAbility.

Te PlayerAbility są reusowalne między przedmiotami, tj. jeśli chcielibyśmy teraz np. dodać drugi pistolet, który wygląda inaczej, zadaje inny dmg, może pomieścić mniej amunicji w magazynku itd., to możemy po prostu stworzyć do tego asset w edytorze (data-only klasę dziedziczącą po UInventoryWeaponDefinition) i podłączyć tę samą, istniejącą już abilitkę strzelania bez potrzeby doimplementowywania niczego nowego.

Pod spodem PlayerAbility zaimplementowałem jako komponenty dodawane do gracza, specjalnie spreparowane, by były replikowalne mimo że są dodawane w runtimie. (Ciekawostka: znalazłem przy tym błąd w silniku w Enhanced Inpucie z nowego unreala, więcej o tym w UEquipmentComponent::ChangeWeaponLocally). Zdecydowałem się na ActorComponenty, a nie zwykłe child Uobjecty, które też mogą być replikowane, ze względu na to, że ActorComponenty działają z unrealowymi inputami out-of-the-box.
Kiedy zmieniamy broń, związane z nią PlayerAbility zostają dodane, a te z poprzedniej broni usunięte.

Klient nasłuchuje inputów w PlayerAbility i wysyła do serwera prośbę o jej aktywację, równocześnie samemu już zaczynając ją symulować. Jeśli serwer się zgodzi, ta abilitka zostaje aktywowana u wszystkich graczy (co może włączać jakieś animacje itd.), a jeśli nie, symulacja u klienta zostaje przerwana - wszystko to jest zaimplementowane w bazowej wersji abilitki.


Bronie
------
Bronie są zdefiniowane jako assety - przedmioty z inventory (data-only blueprintowe dziedziczące po UInventoryWeaponDefinition). Ustawia się tam mesh, liczby amunicji, dmg, i listę PlayerAbility, które dają graczowi, gdy są wyciągnięte. To pozwala na szybkie tworzenie broni, które wykorzystują części istniejącej logiki.


Strzelanie
----------
Strzelanie jest zaimplementowane przez PlayerAbility. Nie zaimplementowałem żadnych animacji związanych ze strzelaniem, zamiast tego zrobiłem je na timerach w przypadku AssaultRifle (BP_Ability_RifleShot / BP_InventoryItem_Weapon_Rifle).

Tu warto by było dodać więcej zabezpieczeń, np. na częstotliwość strzelania - obecnie nie sprawdzam, czy ktoś nie strzela dużo częściej niż dana broń powinna pozwalać.

Replikacja strzelania zawarta jest w klasie UShootingAbility, po której dziedziczą te blueprintowe. By uniknąć hackerów strzelających niewiadomo skąd i trafiających niewiadomo jak, do serwera wysyłane jest tylko skąd jest oddany strzał (co serwer i tak jeszcze waliduje) i w którym kierunku.

Życie, śmierć i respawn
-----------------------
W sumie całą trójką zajmuje się replikowalny UHealthComponent na graczu.

Obecnie obrażenia zadaję tylko na serwerze (w wyniku strzelania) i z serwera są następnie replikowane do pozostałych graczy. Klient nie jest w stanie zadać sobie ani nikomu innemu obrażeń we własnej symulacji.
To może powodować delay, jeśli kogoś postrzelimy, możemy nie zobaczyć, że ta osoba oberwała lub umarła dopóki serwer tego nie potwierdzi. Można by to doimplentować dla lepszej responsywności gry i rollbackować jeśli stan na serwerze jest inny.

Animacje
--------
Pisaliście, że nie będziecie oceniać estetyki strzelania, a spędziłem sporo czasu nad tym zadaniem chcąc oddać przemyślane rozwiązania, więc w końcu zrezygnowałem z implementowania ich.

Jeśli miałbym je implementować, najprawdopodobniej w PlayerAbility użyłbym AnimMontage'y lub linkował/odlinkowywałbym AnimClassLayer jako mini-AnimationBlueprint, który by implementował animacje i blendowanie między nimi dla poszczególnej abilitki.

Jako że sygnały o dodaniu, aktywacji, anulowaniu i usunięciu tych abilitek są replikowane, pozwoliłoby to na odtwarzanie tych samych animacji na wszystkich symulacjach.