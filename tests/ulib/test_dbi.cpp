// test_dbi.cpp

#include <ulib/dbi/dbi.h>

int
U_EXPORT main (int argc, char* argv[])
{
   U_ULIB_INIT(argv);

   U_TRACE(5,"main(%d)", argc)

   UDBI sql;
   UDBIRow r;
   UDBISet res;
   UString name;
   int id, k, n;

   // First we create a sql session object and load the driver "sqlite3" using simple connection string.

   if (sql.setDirectory("./inp/") &&
       sql.connect("test.db")) // ,"root","password","127.0.0.1"))
      {
      // Then we perform queries. First we prepare them using "iostreams like" style: 

      sql << "DROP TABLE IF EXISTS users";

      // and then execute the operation:

      sql.exec();

      // There is a syntactic sugar for this operation:

      sql << "CREATE TABLE users ( id integer primary key not null, name varchar(128) not null )",exec();

      // Then we want to execute some commands using parameters binding:
      //
      // First we load our query. Each "?" represents a bound parameter. Then, using overloaded
      // comma operator we bind actual values: the integer "1" and the string "Moshe".
      //
      // NB: every bound string is automatically escaped.

      sql << "INSERT INTO users(id,name) VALUES(?,?)",1,"Moshe",exec();
      sql << "INSERT INTO users(id,name) VALUES(?,?)",2,"Yossi",exec();

      // Now we want to fetch a single row of data. First, we bind the query and its parameters as before.

      sql << "SELECT name FROM users WHERE id=?",1;

      // But now, we store the output data in a single row class.

      if (sql.single(r))
         {
         // If the result wasn't an empty set, the condition is true and we can readout the data from the row, using the "iostreams" like interface.

         r                 >> name;
         cout << "name = " << name << endl;
         }
      else
         {
         cout << "No user with id=" << 1 << endl;
         }

      // Now, we want to fetch some bigger data set. In this case we use the class result that stores the output data. We use:

      sql << "SELECT id,name FROM users";
      sql.fetch(res);

      // Now we can find out the number of rows calling res.rows() and iterate over each row calling res.next(r)

      while (res.next(r))
         {
         r    >> id         >> name;
         cout << id << "\t" << name << endl;
         }

      cout << "There are " << res.rows() << " users\n";

      double f = -1;
      time_t tt = time(NULL);
      struct tm t = *localtime(&tt);

      sql << "drop table if exists test",exec();
      sql << "create table test ( id integer primary key autoincrement not null,n integer, f real , t timestamp ,name text )",exec();

      cout << asctime(&t);
      sql  << "insert into test(n,f,t,name) values(?,?,?,?)",10,3.1415926565,t,"Hello \'World\'",exec();
      cout << "ID: " << sql.rowid("test_id_seq") << ", Affected rows: " << sql.affected() << endl;

      sql  << "insert into test(n,f,t,name) values(?,?,?,?)",use(10,true),use(3.1415926565,true),use(t,true),use("Hello \'World\'",true),exec();
      cout << "ID: " << sql.rowid("test_id_seq") << ", Affected rows: " << sql.affected() << endl;

      sql  << "select id,n,f,t,name from test limit 10", res;
      cout << "Rows: " << res.rows() << endl;
      cout << "Cols: " << res.cols() << endl;

      (void) name.assign("nonset");

      struct tm atime = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

      for (n = 0, id = -1, k = -1; res.next(r); ++n)
         {
         r >> id >> k >> f >> atime >> name;

         cout << r.get<int>(1) << ' ' << k << ' ' << f << ' ' << name << ' ' << asctime(&atime) << endl;
         cout << "has " << r.cols() << " columns\n";
         }

      sql  << "delete from test where 1<>0",exec();
      cout << "Deleted " << sql.affected() << " rows\n";
      }
}
