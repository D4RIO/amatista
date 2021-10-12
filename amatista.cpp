#include <iostream>                 // IO
#include <fstream>                  // ifstream en BLE
#include <string>                   // string en BLE
#include <unicode/unistr.h>         // ICU Unicode en BLE (https://unicode-org.github.io/icu-docs/apidoc/released/icu4c/classicu_1_1UnicodeString.html)
#include <memory>                   // shared_ptr
#include <vector>
#include <functional>

namespace Amatista {

	class Lista;

	class Objeto {
	public:
		Objeto () {};
		Objeto (std::string n)
			{this->_nombre = n;}

		std::string nombre() const
			{return this->_nombre;}

		void nombre(std::string n)
			{this->_nombre = n;}

		virtual std::ostream& mostrar (std::ostream& salida) const
			{
				salida << "objeto:(" << this->nombre() << ")";
				return salida;
			}

		virtual Lista& ejecutar (Lista&) {throw std::runtime_error ("No es ejecutable");}

		friend std::ostream& operator<< (std::ostream &salida, const Objeto &a)
			{
				return a.mostrar (salida);
			}
	protected:
		std::string _nombre;
	};


	class Entero : public Objeto {

	};

	class Texto : public Objeto {
	public:
		Texto (std::string n, std::string c)
			{this->_nombre = n; this->_c = c;}
		Texto (std::string c)
			{this->_nombre = "anonima"; this->_c = c;}

		std::string contenido (void) const
			{
				return this->_c;
			}
		void contenido (const std::string c)
			{
				this->_c = c;
			}

		std::ostream& mostrar (std::ostream &salida) const
			{
				salida << "texto:(" << this->contenido() << ")";
				return salida;
			}
	private:
		std::string _c;
	};


/** ***************************************************************************
 * Clase Amatista Lista: Representa una lista y sus propiedades. Existen varios
 * objetos Lista, cada uno contiene una expresión en forma de lista. Cada
 * lista es autocontenida y evalúa a un valor.
 ** ***************************************************************************/
	class Lista : public Objeto {
	public:
		Lista ()
			{_n = 0; _linea = 0; _columna = 0;}
		Lista (unsigned int n)
			{_n = n; _linea = 0; _columna = 0;}


		auto vector ()
			{return &_v;}
		auto nivel ()
			{return _n;}
		auto linea ()
			{return _linea;}
		auto columna ()
			{return _columna;}
		auto tamaño ()
			{return this->vector ()->size ();}
		const Objeto& operator[] (std::size_t idx)
			{ return *(this->vector ()->at (idx)); }


		void linea (const unsigned int l)
			{_linea = l;}
		void columna (const unsigned int c)
			{_columna = c;}

		void agregar (const std::shared_ptr<Objeto>& o)
			{this->vector()->push_back(o);}
		void quitarPrimero (void)
			{this->vector()->erase (this->vector()->begin());}

		void mostrarDetalle (void)
			{
				std::cout << "nivel: "<< this->nivel () << " elementos: " << this->vector()->size() << "\n";
				// for ( const auto &i : _v )
				// 	std::cout << *i;
				// std::cout << *this << std::endl;
			}

		std::ostream& mostrar (std::ostream &salida) const
			{
				salida << "lista:( ";
				for ( const auto &elem : _v )
					salida << *elem << " ";
				salida << ")";
				return salida;
			}


		friend std::istream& operator>> (std::istream& entrada, Lista& b)
			{
				b.leerLista (entrada);
				b.ejecutar (b);
				b.mostrarDetalle ();
				return entrada;
			}

		Lista& ejecutar (Lista& l)
			{
				// std::cout << "llamada a Lista.ejecutar ("
				//   << this->linea () << this->columna () << ")" << std::endl;
				return l;
			}

	private:
		std::vector<std::shared_ptr<Objeto>> _v;
		unsigned int _n;
		unsigned int _linea;
		unsigned int _columna;

		std::string leerTexto (std::istream& archivo)
			{
				std::string r;
				char c1;
				while (archivo >> std::noskipws >> c1
					   && c1 != '"'
					   && c1 != '\'')
				{
					_columna++;
					r += c1;
				}
//				std::cout << "DEBUG TEXTO ["<<r<<"]"<<std::endl;
				return r;
			}

		std::string leerSimbolo (std::istream& archivo)
			{
				std::string r;
				char c1;
				while (archivo >> std::noskipws >> c1
					   && c1 != ' '
					   && c1 != '\t'
					   && c1 != '\n')
				{
					_columna++;
					r += c1;
				}
//				std::cout << "DEBUG SIMBOLO ["<<r<<"]"<<std::endl;
				return r;
			}

		void leerLista(std::istream& archivo)
			{
				// std::cout << "DEBUG Ingresando a nivel " << nivel << std::endl;
				char c1;
				while (archivo >> std::noskipws >> c1)
				{
					// std::cout << "DEBUG LEÍDO ["<<c1<<"]"<<std::endl;
					_columna++;
					if (c1 == '(')
					{
						auto l1 = std::make_shared<Lista>(this->nivel()+1);
						l1->leerLista (archivo);
						this->agregar (l1);
					}
					else if (c1 == ')')
					{
						//this->mostrar(nivel);
						break;
					}
					else if (c1 == '"' || c1 == '\'')
					{
						auto s1 = leerTexto (archivo);
						this->agregar (std::make_shared<Texto>(s1));
					}
					else if (c1 != '\n' && c1 != ' ' && c1 != '\t')
					{
						archivo.putback (c1);
						_columna--;
						auto s1 = leerSimbolo (archivo);
						auto o = std::make_shared<Objeto>(s1);
						this->agregar (o);
					}
					else if (c1 == '\n')
					{
						_columna = 0;
						_linea++;
					}
				}
//				std::cout << "DEBUG FUERA DE BUCLE NIVEL " << this->nivel() << std::endl;
			}
	};

	class Funcion : public Objeto {
	public:
		Funcion (const std::string &n, std::function<Lista&(Lista&)> f)
			{this->_nombre = n; this->_f=f;}

		Lista& ejecutar (Lista& params)
			{return _f (params.ejecutar(params));}
	private:
		std::function<Lista&(Lista&)> _f;
	};

	Funcion mostrar ("mostrar",
					   [] (Lista& l) -> Lista& {
						   l.quitarPrimero();
						   if (l.tamaño () == 1) {
							   // TODO este cout debe ser solo en caso que sea texto
							   std::cout << ((Texto&)l[0]).contenido () << std::endl;
						   }
						   else {
							   std::cout << l << std::endl;
						   }
						   return l;
					   });

}

int main (void)
{
	Amatista::Lista loop;
	std::ifstream archivo("test.amt", std::ios::binary);
	archivo >> loop;
	std::cout << "MOSTRANDO" << std::endl;
	Amatista::mostrar.ejecutar ((Amatista::Lista&) loop [0]);
}
