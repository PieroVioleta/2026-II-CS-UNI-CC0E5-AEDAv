#ifndef __EXCEL_H__
#define __EXCEL_H__
#include "../containers/HashTable.h"
#include "../containers/linkedlist.h"
#include <iomanip>

// Excel de texto: celdas "A1" con contenido "5" o "=SUM(A1,B2)*2"
class Excel {
public:
    using Celdas      = HashTable<HashAscTraits<std::string, std::string>>;
    using Valores     = HashTable<HashAscTraits<std::string, Ref>>;
    using ListaReferencias = LinkedList<LinkedListAscTraits<Par<std::string, std::string>>>;

    void Set(const std::string& nombre, const std::string& contenido) {
        m_contenido[nombre] = contenido;
        RegistrarReferencias(contenido, nombre);
        Recalcular(nombre, 0);
    }

    Ref Get(const std::string& nombre) { return m_valores[nombre]; }

    // acepta "A2 5", "A2=5", "A2 = 5", "a2 5"; false si la linea no es valida
    bool SetLinea(const std::string& linea) {
        std::string texto = Mayusculas(linea);
        std::string nombre, contenido;
        size_t eq = texto.find('=');
        if (eq != std::string::npos && eq > 0 && texto[eq - 1] != ' ') {
            nombre    = texto.substr(0, eq);        // "A2=1": el = es el separador
            contenido = texto.substr(eq + 1);
        } else if (eq != std::string::npos) {
            nombre    = texto.substr(0, eq);        // "B1 =A2*3": el = pertenece a la formula
            contenido = texto.substr(eq);
        } else {
            size_t sep = texto.find(' ');
            if (sep == std::string::npos)
                return false;
            nombre    = texto.substr(0, sep);
            contenido = texto.substr(sep + 1);
        }
        nombre    = Limpiar(nombre);
        contenido = Limpiar(contenido);
        if (!EsCelda(nombre))
            return false;
        Set(nombre, contenido);
        return true;
    }

    static std::string Mayusculas(std::string s) {
        for (auto& c : s)
            if (c >= 'a' && c <= 'z')
                c = c - 'a' + 'A';
        return s;
    }

    static std::string Limpiar(const std::string& s) {
        size_t a = s.find_first_not_of(' ');
        if (a == std::string::npos)
            return "";
        size_t b = s.find_last_not_of(' ');
        return s.substr(a, b - a + 1);
    }

    friend std::ostream& operator<<(std::ostream& os, const Excel& e) {
        for (auto it = e.m_contenido.begin(); it != e.m_contenido.end(); ++it)
            os << (*it).getValue().first << "=" << (*it).getValue().second << "\n";
        return os;
    }

    friend std::istream& operator>>(std::istream& is, Excel& e) {
        std::string linea;
        while (std::getline(is, linea))
            e.SetLinea(linea);
        return is;
    }

    void PrintTree() const { m_valores.PrintTree(); }

    const Celdas& Contenido() const { return m_contenido; }

    const ListaReferencias& Referencias() const { return m_referencias; }

    void ImprimirCuadricula(std::ostream& os, TX filas, const std::string& letras) const {
        std::string techo = "      ┌", medio = "      ├", piso = "      └";
        for (size_t i = 0; i < letras.size(); ++i) {
            techo += "────────" + std::string(i + 1 == letras.size() ? "┐" : "┬");
            medio  += "────────" + std::string(i + 1 == letras.size() ? "┤" : "┼");
            piso   += "────────" + std::string(i + 1 == letras.size() ? "┘" : "┴");
        }
        os << techo << "\n      │";
        for (auto c : letras)
            os << "   " << c << "    │";
        os << "\n";
        for (TX f = 1; f <= filas; ++f) {
            os << medio << "\n  " << f << "  │";
            for (auto c : letras) {
                auto n = m_valores.Buscar(std::string(1, c) + std::to_string(f));
                if (n)
                    os << " " << std::setw(6) << n->getValue().second << " │";
                else
                    os << "        │";
            }
            os << "\n";
        }
        os << piso << "\n";
    }

private:
    Celdas m_contenido;
    Valores m_valores;
    ListaReferencias m_referencias;

    static Ref ANumero(const std::string& s) {
        Ref v = 0;
        for (auto c : s) {
            if (c < '0' || c > '9') break;
            v = v * 10 + (c - '0');
        }
        return v;
    }

    void RegistrarReferencias(const std::string& formula, const std::string& origen) {
        for (size_t i = 0; i + 1 < formula.size(); ++i) {
            auto c = formula[i];
            if (c >= 'A' && c <= 'Z' && formula[i+1] >= '0' && formula[i+1] <= '9') {
                size_t j = i + 1;
                while (j < formula.size() && formula[j] >= '0' && formula[j] <= '9') ++j;
                m_referencias.push_back(Par<std::string, std::string>{formula.substr(i, j - i), origen}, 0);
                i = j;
            }
        }
    }

    static bool EsCelda(const std::string& s) {
        size_t i = 0;
        while (i < s.size() && s[i] >= 'A' && s[i] <= 'Z') ++i;
        if (i == 0 || i == s.size())
            return false;
        while (i < s.size() && s[i] >= '0' && s[i] <= '9') ++i;
        return i == s.size();
    }

    void Recalcular(const std::string& nombre, TX depth) {
        if (depth > 100) return;
        Evaluar(nombre);
        for (auto it = m_referencias.begin(); it != m_referencias.end(); ++it)
            if ((*it).getValue().first == nombre)
                Recalcular((*it).getValue().second, depth + 1);
    }

    void Evaluar(const std::string& nombre) {
        std::string f = m_contenido[nombre];
        size_t pos = 0;
        if (!f.empty() && f[0] == '=')
            m_valores[nombre] = EvalExpr(f, ++pos);
        else
            m_valores[nombre] = ANumero(f);
    }

    static void Saltar(const std::string& f, size_t& pos) {
        while (pos < f.size() && f[pos] == ' ')
            ++pos;
    }

    Ref EvalExpr(const std::string& f, size_t& pos) {
        Saltar(f, pos);
        Ref v = EvalTermino(f, pos);
        for (Saltar(f, pos); pos < f.size() && (f[pos] == '+' || f[pos] == '-'); Saltar(f, pos)) {
            auto op = f[pos++];
            Ref r = EvalTermino(f, pos);
            if (op == '+') v += r;
            else v -= r;
        }
        return v;
    }

    Ref EvalTermino(const std::string& f, size_t& pos) {
        Ref v = EvalFactor(f, pos);
        for (Saltar(f, pos); pos < f.size() && (f[pos] == '*' || f[pos] == '/'); Saltar(f, pos)) {
            auto op = f[pos++];
            Ref r = EvalFactor(f, pos);
            if (op == '*') v *= r;
            else if (r) v /= r;
            else v = 0;
        }
        return v;
    }

    Ref EvalFactor(const std::string& f, size_t& pos) {
        Saltar(f, pos);
        if (pos >= f.size()) return 0;
        if (f[pos] == '(') {
            ++pos;
            Ref v = EvalExpr(f, pos);
            Saltar(f, pos);
            if (pos < f.size() && f[pos] == ')') ++pos;
            return v;
        }
        if (f[pos] >= '0' && f[pos] <= '9') {
            Ref v = 0;
            while (pos < f.size() && f[pos] >= '0' && f[pos] <= '9')
                v = v * 10 + (f[pos++] - '0');
            return v;
        }
        if (f[pos] >= 'A' && f[pos] <= 'Z') {
            size_t ini = pos;
            while (pos < f.size() && f[pos] >= 'A' && f[pos] <= 'Z') ++pos;
            bool es_funcion = pos < f.size() && f[pos] == '(';
            if (!es_funcion)
                while (pos < f.size() && f[pos] >= '0' && f[pos] <= '9') ++pos;
            std::string nombre = f.substr(ini, pos - ini);
            if (es_funcion)
                return EvalFuncion(nombre, f, pos);
            return m_valores[nombre];
        }
        return 0;
    }

    Ref EvalFuncion(const std::string& fn, const std::string& f, size_t& pos) {
        ++pos;
        Ref sum = 0, cuenta = 0, mx = 0, mn = 0;
        bool primero = true;
        for (Saltar(f, pos); pos < f.size() && f[pos] != ')'; Saltar(f, pos)) {
            Ref v = EvalExpr(f, pos);
            sum += v;
            ++cuenta;
            if (primero || v > mx) mx = v;
            if (primero || v < mn) mn = v;
            primero = false;
            Saltar(f, pos);
            if (pos < f.size() && f[pos] == ',') ++pos;
        }
        if (pos < f.size()) ++pos;
        if (fn == "SUM") return sum;
        if (fn == "AVG") return cuenta ? sum / cuenta : 0;
        if (fn == "MAX") return mx;
        if (fn == "MIN") return mn;
        return 0;
    }
};

#endif // __EXCEL_H__
