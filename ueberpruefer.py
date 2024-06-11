eingabe = input()
inhalt = eingabe.split(" ")

check = input()
for item in check.split(" "):
    if not item in inhalt:
        print (item)