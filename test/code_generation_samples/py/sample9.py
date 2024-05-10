class Human:
    class __Human:
        def __init__(self, surname):
            self.surname = surname
        
    class Men:
        def __init__(self, surname):
            self.surname = surname

    def __init__(self, name) -> None:
        self.name = name

h = Human.Men("Krasnov")
print(h)
