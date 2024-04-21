from message_2 import *

adr = Address(
  id=1,
  street="Street",
  floor=-1,
  withIntercom=False,
)
print(adr)

evgeny = User(
  id=1,
  name="Evgeny",
  address=adr
)

print(evgeny)