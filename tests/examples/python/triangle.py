from math import sqrt
print('Entrer du triangle isocele du sommet A le 2 cotes a = BC et b = AC = AB:')
str = input('a = ')
a = float(str)
str = input('b = ')
b = float(str)
h = sqrt(a*a - (b*b /4))
print("l'aire du triangle isocele, ayant base {0} et hauteur {1} il est: {2}".format(a, h, (a*h)/2))
