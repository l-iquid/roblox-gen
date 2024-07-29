class Person:
  def __init__(self, name, age):
    self.name = name
    self.age = age
  def say_name(self):
    print("I am " + self.name)

p1 = Person("John", 36)

print(p1.name)
print(p1.age)