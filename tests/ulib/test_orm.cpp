// test_orm.cpp

#include <ulib/orm/orm.h>
#include <ulib/orm/orm_driver.h>

#ifdef DEBUG
#  include <ulib/internal/objectIO.h>
#endif

#include <vector>

class Person {
public:
   // Check for memory error
   U_MEMORY_TEST

   // Allocator e Deallocator
   U_MEMORY_ALLOCATOR
   U_MEMORY_DEALLOCATOR

   UString _lastName;
   UString _firstName;
   UString _address;
   int _age;

   Person()
      {
      U_TRACE_CTOR(5, Person, "")

      _age = 0;
      }

   Person(const UString& ln, const UString& fn, const UString& adr, int a) : _lastName(ln), _firstName(fn), _address(adr), _age(a)
      {
      U_TRACE_CTOR(5, Person, "%.*S,%.*S,%.*S,%u", U_STRING_TO_TRACE(ln), U_STRING_TO_TRACE(fn), U_STRING_TO_TRACE(adr), a)
      }

   ~Person()
      {
      U_TRACE_DTOR(5, Person)
      }

   bool operator==(const Person& other) const
      {
      return  _lastName == other._lastName  &&
             _firstName == other._firstName &&
               _address == other._address   &&
                   _age == other._age;
      }

   bool operator<(const Person& p) const
      {
      if (      _age < p._age      ||
           _lastName < p._lastName ||
          _firstName < p._firstName)
         {
         return true;
         }

      return (_address < p._address);
      }

   void bindParam(UOrmStatement* stmt)
      {
      U_TRACE(0, "Person::bindParam(%p)", stmt)

      // the table is defined as Person (LastName VARCHAR(30), FirstName VARCHAR(30), Address VARCHAR(30), Age INTEGER)

      stmt->bindParam(U_ORM_TYPE_HANDLER(_lastName,  UString));
      stmt->bindParam(U_ORM_TYPE_HANDLER(_firstName, UString));
      stmt->bindParam(U_ORM_TYPE_HANDLER(_address,   UString));
      stmt->bindParam(U_ORM_TYPE_HANDLER(_age,       int));
      }

   void bindResult(UOrmStatement* stmt)
      {
      U_TRACE(0, "Person::bindResult(%p)", stmt)

      // the table is defined as Person (LastName VARCHAR(30), FirstName VARCHAR(30), Address VARCHAR(30), Age INTEGER)

      stmt->bindResult(U_ORM_TYPE_HANDLER(_lastName,  UString));
      stmt->bindResult(U_ORM_TYPE_HANDLER(_firstName, UString));
      stmt->bindResult(U_ORM_TYPE_HANDLER(_address,   UString));
      stmt->bindResult(U_ORM_TYPE_HANDLER(_age,       int));
      }

#ifdef DEBUG
   const char* dump(bool breset) const
      {
      *UObjectIO::os << "_age               " << _age               << '\n'
                     << "_address   (UString" << (void*)&_address   << ")\n"
                     << "_lastName  (UString" << (void*)&_lastName  << ")\n"
                     << "_firstName (UString" << (void*)&_firstName << ')';

      if (breset)
         {
         UObjectIO::output();

         return UObjectIO::buffer_output;
         }

      return U_NULLPTR;
      }
#endif

private:
   Person& operator=(const Person&) { return *this; }
};

class Test1 {
public:
   // Check for memory error
   U_MEMORY_TEST

   // Allocator e Deallocator
   U_MEMORY_ALLOCATOR
   U_MEMORY_DEALLOCATOR

   int id;
   UString name;

   // CONSTRUCTOR

   Test1()
      {
      U_TRACE_CTOR(5, Test1, "")
      }

   Test1(const Test1& t)
      {
      U_TRACE_CTOR(5, Test1, "%p", &t)

      U_MEMORY_TEST_COPY(t)

      id   = t.id;
      name = t.name;
      }

   ~Test1()
      {
      U_TRACE_DTOR(5, Test1)
      }

   void bindParam(UOrmStatement* stmt)
      {
      U_TRACE(0, "UOrmTypeHandler<Test1>::bindParam(%p)", stmt)

      stmt->bindParam(U_ORM_TYPE_HANDLER(id,   int));
      stmt->bindParam(U_ORM_TYPE_HANDLER(name, UString));
      }

   void bindResult(UOrmStatement* stmt)
      {
      U_TRACE(0, "UOrmTypeHandler<Test1>::bindResult(%p)", stmt)

      stmt->bindResult(U_ORM_TYPE_HANDLER(id,   int));
      stmt->bindResult(U_ORM_TYPE_HANDLER(name, UString));
      }

#ifdef DEBUG
   const char* dump(bool breset) const
      {
      *UObjectIO::os << "id           " << id           << '\n'
                     << "name (UString" << (void*)&name << ')';

      if (breset)
         {
         UObjectIO::output();

         return UObjectIO::buffer_output;
         }

      return U_NULLPTR;
      }
#endif

private:
   Test1& operator=(const Test1&) { return *this; }
};

static void testBinding(UOrmSession* sql)
{
   U_TRACE(5, "testBinding(%p)", sql)

   UString lastName("Simpson");
   UString firstName("Bart");
   UString address("Springfield");
   int age = 12;

   int count = 0;
   UString result;

   /*
   UString tableName("Person");
   UOrmStatement select1(*sql, U_CONSTANT_TO_PARAM("SELECT name FROM sqlite_master WHERE tbl_name = ?"));

   select1.use(tableName);
   select1.into(result);

   select1.execute();

   U_ASSERT(result == tableName)
   */

   UOrmStatement insert(*sql, U_CONSTANT_TO_PARAM("INSERT INTO Person VALUES(?, ?, ?, ?)"));

   // -----------------------------------------------------------------------------------------------------------------
   // following should not compile:
   // -----------------------------------------------------------------------------------------------------------------
   // insert.use("Simpson", "Bart", "Springfield", age);
   // -----------------------------------------------------------------------------------------------------------------

   insert.use(lastName, firstName, address, age);

   insert.execute();

   insert.execute();

   UOrmStatement select2(*sql, U_CONSTANT_TO_PARAM("SELECT COUNT(*) FROM Person"));

   select2.into(count);

   select2.execute();

   U_INTERNAL_DUMP("count = %d", count)

// U_INTERNAL_ASSERT(count == 2)
}

static void testSimpleAccess(UOrmSession* sql)
{
   U_TRACE(5, "testSimpleAccess(%p)", sql)

   UString lastName("lastname");
   UString firstName("firstname");
   UString address("Address");
   int age = 133132;
   int count = 0;
   UString result;

   /*
   UString tableName("Person");
   UOrmStatement select1(*sql, U_CONSTANT_TO_PARAM("SELECT name FROM sqlite_master WHERE tbl_name = ?"));

   select1.use(tableName);
   select1.into(result);

   select1.execute();

   U_ASSERT(result == tableName)
   */

   UOrmStatement insert(*sql, U_CONSTANT_TO_PARAM("INSERT INTO Person VALUES(?, ?, ?, ?)"));

   insert.use(lastName, firstName, address, age);

   insert.execute();

   UOrmStatement select2(*sql, U_CONSTANT_TO_PARAM("SELECT COUNT(*) FROM Person"));

   select2.into(count);

   select2.execute();

   U_INTERNAL_DUMP("count = %d", count)

// U_INTERNAL_ASSERT(count == 3)

   UOrmStatement select3(*sql, U_CONSTANT_TO_PARAM("SELECT LastName FROM Person WHERE LASTNAME = ?"));

   select3.into(result);
   select3.use(lastName);

   select3.execute();

   U_ASSERT(lastName == result)

   UOrmStatement select4(*sql, U_CONSTANT_TO_PARAM("SELECT Age FROM Person WHERE LASTNAME = ?"));

   select4.into(count);
   select4.use(lastName);

   select4.execute();

   U_INTERNAL_ASSERT(count == age)

   *sql << "UPDATE Person SET Age = -1";

   select4.execute();

   U_INTERNAL_ASSERT(-1 == count)
}

static void testSimpleAccessVector(UOrmSession* sql)
{
   U_TRACE(5, "testSimpleAccessVector(%p)", sql)

   UVector<UString> vec;
   vec.push_back(U_STRING_FROM_CONSTANT("LN1"));
   vec.push_back(U_STRING_FROM_CONSTANT("FN1"));
   vec.push_back(U_STRING_FROM_CONSTANT("ADDR1"));
   int count = 0;

   UOrmStatement insert(*sql, U_CONSTANT_TO_PARAM("INSERT INTO PersonVec VALUES(?, ?, ?)"));

   insert.use(vec);

   insert.execute();

   UOrmStatement select1(*sql, U_CONSTANT_TO_PARAM("SELECT COUNT(*) FROM PersonVec"));

   select1.into(count);

   select1.execute();

// U_INTERNAL_ASSERT(count == 1)

   UVector<UString> vecR;
   vecR.push_back(UString(100U));
   vecR.push_back(UString(100U));
   vecR.push_back(UString(100U));

   UOrmStatement select2(*sql, U_CONSTANT_TO_PARAM("SELECT * FROM PersonVec"));

   select2.into(vecR);

   select2.execute();

   U_ASSERT(vec == vecR)
}

static void testComplexType(UOrmSession* sql)
{
   U_TRACE(5, "testComplexType(%p)", sql)

   Person p1(U_STRING_FROM_CONSTANT("LN1"), U_STRING_FROM_CONSTANT("FN1"), U_STRING_FROM_CONSTANT("ADDR1"), 1);
   Person p2(U_STRING_FROM_CONSTANT("LN2"), U_STRING_FROM_CONSTANT("FN2"), U_STRING_FROM_CONSTANT("ADDR2"), 2);

   UOrmStatement insert(*sql, U_CONSTANT_TO_PARAM("INSERT INTO Person VALUES(?, ?, ?, ?)"));

   insert.use(p1);

   insert.execute();

   insert.reset();

   insert.use(p2);

   insert.execute();

   int count = 0;

   UOrmStatement select1(*sql, U_CONSTANT_TO_PARAM("SELECT COUNT(*) FROM Person"));

   select1.into(count);

   select1.execute();

// U_INTERNAL_ASSERT(count == 5)

   Person c1, c2;

   UOrmStatement select2(*sql, U_CONSTANT_TO_PARAM("SELECT * FROM Person WHERE LASTNAME = ?"));

   select2.use(p1._lastName);
   select2.into(c1);

   select2.execute();

   U_ASSERT(c1 == p1)

   select2.reset();

   select2.use(p2._lastName);
   select2.into(c2);

   select2.execute();

   U_INTERNAL_DUMP("p2._lastName = %.*S c2._lastName = %.*S", U_STRING_TO_TRACE(p2._lastName), U_STRING_TO_TRACE(c2._lastName))

   U_ASSERT(p2 == c2)
}

#define  PGSQL_AUTO_INCREMENT "serial  primary key"
#define  MYSQL_AUTO_INCREMENT "integer primary key auto_increment"
#define SQLITE_AUTO_INCREMENT "integer primary key autoincrement"

//#define AUTO_INCREMENT  PGSQL_AUTO_INCREMENT
//#define AUTO_INCREMENT  MYSQL_AUTO_INCREMENT
  #define AUTO_INCREMENT SQLITE_AUTO_INCREMENT

int
U_EXPORT main(int argc, char* argv[])
{
   U_ULIB_INIT(argv);

   U_TRACE(5,"main(%d)", argc)

   UString orm_driver_dir( argv[1]);
   UString orm_driver_list(argv[2]);

   // First we load the drivers

   if (UOrmDriver::loadDriver(orm_driver_dir, orm_driver_list) == false) U_ERROR("ORM drivers load failed. Going down...");

   u_atexit(UOrmDriver::clear);

   // Then we create a sql session object using the connection string

   UOrmSession sql(argv[3], strlen(argv[3]), UString(argv[4]));

   // Then we perform queries. First we prepare them using "iostreams like" style and then execute the operation:

   sql << "DROP   TABLE IF     EXISTS test";
   sql << "DROP   TABLE IF     EXISTS users";
   sql << "DROP   TABLE IF     EXISTS Person";
   sql << "DROP   TABLE IF     EXISTS PersonVec";
   sql << "CREATE TABLE IF NOT EXISTS users (id INTEGER PRIMARY KEY NOT NULL, name VARCHAR(128) NOT NULL)";
   sql << "CREATE TABLE IF NOT EXISTS PersonVec (LastName VARCHAR(30), FirstName VARCHAR(30), Address VARCHAR(30))";
   sql << "CREATE TABLE IF NOT EXISTS Person (LastName VARCHAR(30), FirstName VARCHAR(30), Address VARCHAR(30), Age INTEGER)";
   sql << "CREATE TABLE IF NOT EXISTS test (id " AUTO_INCREMENT " not null, n integer, f real, t INTEGER, name text)";

   // Then we want to execute some commands using parameters binding. First we load our query. Each "?" represents a bound parameter

   UOrmStatement select2(sql, U_CONSTANT_TO_PARAM("SELECT id, name FROM users")),
                 select1(sql, U_CONSTANT_TO_PARAM("SELECT name FROM users WHERE id = ?")),
                 insert1(sql, U_CONSTANT_TO_PARAM("INSERT INTO users(id, name) VALUES(?, ?)")),
                 select3(sql, U_CONSTANT_TO_PARAM("SELECT id, n, f, t, name from test limit 10")),
                 insert2(sql, U_CONSTANT_TO_PARAM("INSERT INTO test(n, f, t, name) values(?, ?, ?, ?)"));

   // Then we bind actual values: the integer "1" and the string "Moshe"

   int value1      = 1;
   const char* str = "Moshe";

   insert1.use(value1, str);

   insert1.execute();

   // After we bind actual values: the integer "2" and the string "Yossi"

   value1 = 2;

   insert1.bindParam(U_CONSTANT_TO_PARAM("Yossi"), true, 1); // rebind value at column 1 (starting from 0)

   insert1.execute();

   // Now we want to fetch a single row of data. We bind the query and its parameters as before.

   value1 = 1;
   UString name;

   select1.use(value1);
   select1.into(name);

   select1.execute();

   cout << "name = " << name << endl;

   // Now, we want to fetch some bigger data set. In this case we use the class Test1 that stores the output data. We use:

   Test1 t1;

   select2.into(t1);

   select2.execute();

   // Now we can find out the number of rows and iterate over each row

   do {
      cout << t1.id   << "\t"
           << t1.name << endl;
      }
   while (select2.nextRow());

   cout << "There are " << select2.affected() << " users\n";

   value1 = 10;
   str = "Hello 'World'";
   float f = 3.1415926565;
   time_t tt = time(U_NULLPTR);
   struct tm t = *localtime(&tt);

   cout << asctime(&t);

// tt = 0; POSTGRES FAILED !!!!

   insert2.use(value1, f, tt, str);

   insert2.execute();

   cout << "ID: " << insert2.last_insert_rowid("test_id_seq") << ", Affected rows: " << insert2.affected() << endl;

   insert2.execute();

   cout << "ID: " << insert2.last_insert_rowid("test_id_seq") << ", Affected rows: " << insert2.affected() << endl;

   f = -1;
   tt = 0;
   int id = 0, n = 0;
   t = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
   (void) name.assign(U_CONSTANT_TO_PARAM("nonset"));

   select3.into(id, n, f, tt, name);

   select3.execute();

   cout << "Rows: " << select3.affected() << endl;
   cout << "Cols: " << select3.cols()     << endl;

   do {
      (void) U_SYSCALL(gmtime_r, "%p,%p", &tt, &t);

      cout << id << ' ' << n << ' ' << f << ' ' << name << ' ' << asctime(&t) << endl;
      cout << "has " << select3.cols() << " columns\n";
      }
   while (select3.nextRow());

   sql << "DELETE FROM test WHERE 1<>0";

   cout << "Deleted " << sql.affected() << " rows\n";

   testBinding(&sql);
   testSimpleAccess(&sql);
   testSimpleAccessVector(&sql);
   testComplexType(&sql);
}
