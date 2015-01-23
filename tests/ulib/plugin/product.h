// product.h

#ifndef product_H
#define product_H

#include <ulib/dynamic/plugin.h>

class Product {
public:
				Product() {}
	virtual ~Product() { (void) write(1, U_CONSTANT_TO_PARAM("distruttore Product\n")); }

	virtual void print(ostream& os) const = 0;

	friend ostream& operator<<(ostream& os, const Product& base);
};

ostream& operator<<(ostream& os, const Product& base) { base.print(os); return os; }

#endif
