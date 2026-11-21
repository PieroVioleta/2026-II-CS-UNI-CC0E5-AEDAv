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

    friend std::ostream& operator<<(std::ostream& os, const Excel& e) {
        os << e.m_contenido;
        return os;
    }

    friend std::istream& operator>>(std::istream& is, Excel& e) {
        is >> e.m_contenido;
        for (auto it = e.m_contenido.begin(); it != e.m_contenido.end(); ++it)
            e.RegistrarReferencias((*it).getValue().second, (*it).getValue().first);
        for (auto it = e.m_contenido.begin(); it != e.m_contenido.end(); ++it)
            e.Recalcular((*it).getValue().first, 0);
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

    Ref EvalExpr(const std::string& f, size_t& pos) {
        Ref v = EvalTermino(f, pos);
        while (pos < f.size() && (f[pos] == '+' || f[pos] == '-')) {
            auto op = f[pos++];
            Ref r = EvalTermino(f, pos);
            if (op == '+') v += r;
            else v -= r;
        }
        return v;
    }

    Ref EvalTermino(const std::string& f, size_t& pos) {
        Ref v = EvalFactor(f, pos);
        while (pos < f.size() && (f[pos] == '*' || f[pos] == '/')) {
            auto op = f[pos++];
            Ref r = EvalFactor(f, pos);
            if (op == '*') v *= r;
            else if (r) v /= r;
            else v = 0;
        }
        return v;
    }

    Ref EvalFactor(const std::string& f, size_t& pos) {
        if (pos >= f.size()) return 0;
        if (f[pos] == '(') {
            ++pos;
            Ref v = EvalExpr(f, pos);
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
        while (pos < f.size() && f[pos] != ')') {
            Ref v = EvalExpr(f, pos);
            sum += v;
            ++cuenta;
            if (primero || v > mx) mx = v;
            if (primero || v < mn) mn = v;
            primero = false;
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
