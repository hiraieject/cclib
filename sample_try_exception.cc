/* -*- Mode: C++; tab-width: 8; c-basic-offset: 4 -*- */

// throu の引数
// 
// std::exception:
// すべての標準例外クラスの基底クラスです。通常、whatメソッドをオーバーライドしてエラーメッセージを提供します。
// 基底クラスなので、通常は使用しない
// 
// std::runtime_error:
// ランタイムエラーを表すクラスです。プログラムの実行中に発生するエラーに使用されます。
// 
// std::logic_error:
// ロジックエラーを表すクラスです。プログラムの論理的なエラーに使用されます。
// 
// std::bad_alloc:
// メモリ割り当てに失敗した場合にスローされるクラスです。通常、new演算子が失敗したときにこの例外が発生します。
// 
// std::invalid_argument:
// 関数に渡された引数が無効な場合にスローされるクラスです。
// 
// std::out_of_range:
// インデックスや範囲が有効範囲外である場合にスローされるクラスです。通常、コンテナの要素へのアクセス時に使用されます。
// 
// std::length_error:
// 文字列やコンテナの長さが許容範囲を超えた場合にスローされるクラスです。
// 
// std::overflow_error:
// 算術演算によるオーバーフローが発生した場合にスローされるクラスです。
// 
// std::underflow_error:
// 算術演算によるアンダーフローが発生した場合にスローされるクラスです。
// 
// std::domain_error:
// 関数のドメイン（定義域）違反が発生した場合にスローされるクラスです。
// 
// std::range_error:
// 数値の範囲エラーが発生した場合にスローされるクラスです。

#include <stdexcept>
#include <iostream>

extern "C" {
#include <getopt.h>
}

class TestClass {
public:
    TestClass(void) {
        printf ("TestClass constructor\n");
    }
    ~TestClass() {
        printf ("TestClass destructor\n");
    }
};

class CustomException : public std::exception {
public:
    CustomException (const char* message) : errorMessage(message) {
    }
    virtual const char* what() const throw() {
        return errorMessage.c_str();
    }
private:
    std::string errorMessage;
};

int
main (int argc, char *argv[])
{
    int exitcode = 0;

    try {
        TestClass test_instance; // exceptionで抜けたときのインスタンス開放テスト

        bool option_C = false;
        
        int opt;
        while ((opt = getopt(argc, argv, "rlbioLOudRcC")) != -1) {
            switch (opt) {
            case 'r':
                throw std::runtime_error("runtime_error test");
                break;
            case 'l':
                throw std::logic_error("logic_error test");
                break;
            case 'b':
                throw std::bad_alloc();
                break;
            case 'i':
                throw std::invalid_argument("invalid_argument test");
                break;
            case 'o':
                throw std::out_of_range("out_of_range test");
                break;
            case 'L':
                throw std::length_error("length_error test");
                break;
            case 'O':
                throw std::overflow_error("overflow_error test");
                break;
            case 'u':
                throw std::underflow_error("underflow_error test");
                break;
            case 'd':
                throw std::domain_error("domain_error test");
                break;
            case 'R':
                throw std::range_error("range_error test");
                break;
            case 'c':
                throw CustomException("custom exception test");
                break;
            case 'C':
                option_C = true;
                break;
            }
        }

        // try のネストテスト
        try {
            if (option_C) {
                throw CustomException("nested exception test");
            }
        } catch (CustomException& e) {
            std::cout << "  catch nested exception [" << e.what() << "]" << std::endl;
            // exit(1);             // X ここで exit すると test_instance のデストラクタは呼ばれない
            // goto ENDEND;         // X goto で抜けても同じ、test_instance のデストラクタは呼ばれない
            exitcode = 1;           // O 変数をつかって抜けるとデストラクタが呼ばれる
        }

        // ライブラリの実装上exceptionをcatchする順番に成約があり、エラーになる場合がある
        // 以下は、エラーが出ないように並べ替えている
        
    } catch (const std::overflow_error& e) {
        std::cout << "  catch overflow error exception [" << e.what() << "]" << std::endl;
        exit(1);
    } catch (const std::underflow_error& e) {
        std::cout << "  catch under error exception [" << e.what() << "]" << std::endl;
        exit(1);
    } catch (const std::range_error& e) {
        std::cout << "  catch range error exception [" << e.what() << "]" << std::endl;
        exit(1);
    } catch (const std::out_of_range& e) {
        std::cout << "  catch outof range exception [" << e.what() << "]" << std::endl;
        exit(1);
    } catch (const std::length_error& e) {
        std::cout << "  catch length error exception [" << e.what() << "]" << std::endl;
        exit(1);
    } catch (const std::domain_error& e) {
        std::cout << "  catch domain error exception [" << e.what() << "]" << std::endl;
        exit(1);
    } catch (const std::invalid_argument& e) {
        std::cout << "  catch invalid argment exception [" << e.what() << "]" << std::endl;
        exit(1);
    } catch (const std::runtime_error& e) {
        std::cout << "  catch runtime error exception [" << e.what() << "]" << std::endl;
        exit(1);
    } catch (const std::logic_error& e) {
        std::cout << "  catch logic exception [" << e.what() << "]" << std::endl;
        exit(1);
    } catch (const std::bad_alloc& e) {
        std::cout << "  catch bad alloc exception [" << e.what() << "]" << std::endl;
        exit(1);

    } catch (CustomException& e) {
        std::cout << "  catch custom exception [" << e.what() << "]" << std::endl;
        exit(1);
        
    }
//ENDEND:
    if (exitcode == 0) {
        std::cout << "正常終了しました" << std::endl;
    }
    return exitcode;
}
