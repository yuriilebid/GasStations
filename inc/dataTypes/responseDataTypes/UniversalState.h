#ifndef PTS2_0_UNIVERSALSTATE_H
#define PTS2_0_UNIVERSALSTATE_H

#include <string>

enum class Sender {
    FISCAL,
    JSON,
    PTS
};

enum class StateType : int {
    /// 0-6: Комманды имеют одинаковый значения по flow_of_dispensing, то есть на них надо выводить состояние пистолета
    BUSY,                        ///         -- Используется для пометки использоваиня
    NOT_SERVED,                  /// MRS     -- Данный пистолет не обслуживается (запрещнный)
    HANGED_FREE,                 /// MRS     -- Пистолеты повешены, ожидание снятия пистолета
    REMOVED_WAITING,             /// ADAST   -- Пистолет снят, идет таймаут на получения заказа (A_TRANS_ENABLE)
    REMOVED_FREE,                /// MRS     -- Пистолет снят, ожидание заказа
    WAITING_APPROPRIATE_PISTOL,  /// UNIPUMP -- Ожидание поднятия нужного пистолета
    REMOVED_MANUAL_DOSING,       /// MRS     -- Пистолет снят, происходит набор дозы в ручном режиме с клавиатуры КК (после пуска  с КК будет произведена транзакция  «ДО полного бака»)
    AUTHORIZE_REGISTERED,
    /// 7-11: Комманды имеют одинаковый значения по flow_of_dispensing, то есть на них надо делать вывод отпущенного топлива
    TEST_INDICATORS,             /// MRS     -- Заказ получен, происходит обработка заказа (тест индикатора)
    CUTOFF_FUEL_ACTIVE,          /// SHELF   -- Отсечной клапан активен
    FUEL_SUPPLY,                 /// MRS     -- Происходит отпуск топлива
    SLOWING_FUEL_ACTIVE,         /// SHELF   -- Замедляющий клапан активен
    FUEL_ENGINE_ACTIVE,          /// SHELF   -- Для данной стороны двигатель данного вида топлива в работе
    /// 12-19: Комманды имеют одинаковый значения по flow_of_dispensing, то есть после них надо делать закрытие транзакции
    SUPPLY_STOPPED_SC,           /// MRS     -- Отпуск приостановлен по команде СУ (Системы управления), SC (System control)
    SUPPLY_STOPPED,              /// SHELF   -- Отпуск приостановлен
    SUPPLY_STOPPED_NO_PULSE,     /// MRS     -- Отпуск приостановлен - отсутствуют импульсы от датчика ТРК
    SUPPLY_STOPPED_NEST,         /// MRS     -- Отпуск приостановлен – пистолет возвращен в гнездо ТРК
    SUPPLY_STOPPED_NO_POWER,     /// MRS     -- Отпуск приостановлен – пропадание питания
    SUPPLY_DONE,                 /// MRS     -- Отпуск завершен, выбег (длительность этого режима  программируется в КК, рекомендовано для большинства ТРК -3с)
    SUPPLY_DONE_REPORT_OPEN,     /// MRS     -- Отпуск завершен, отчет не закрыт
    WAITING_CONFIRM_EERROR,      /// ADAST   -- Ожидание подтверждения транзакции после E-error (timeout)
    /// 20: Состояние после налива и закрытия, нужно писать состояние END_OF_DISPENSING на JSON пока пистоле не прийдет в одну из первый стадий
    SUPPLY_DONE_REPORT_CLOSE,    /// MRS     -- Отпуск завершен, отчет закрыт, пистолет не повешен
    LOCK_STATUS,
    TOTAL_RESPONSE,
    TRANSACTION_ClOSED,
    TRANSACTION_UNClOSED,
    /// 23: Ошибка по неизвестной причине
    ERROR                        /// UNIPUMP -- Случай ошибки ответа статуса
};

struct Product {
    int price;
    std::string name;
};

struct ProductVolume {
    int volume;
};

using ColumnId = int;

class UniversalState {
private:
    int columnId{};
    int activeNozzle{};
    int dbId{};
    StateType type {};
    std::string controlUser;
public:
    explicit UniversalState() : columnId(0), activeNozzle(0), type(StateType::HANGED_FREE), dbId(0) {
        setType(StateType::FUEL_SUPPLY);
    };
    explicit UniversalState(int _columnId, int _activeNozzle, StateType _type, int _dbId);
    int getId() const;
    int getNozzle() const;
    int getDbId() const;
    StateType getType() const;
    std::string getControlUser();

    void setId(ColumnId newId);
    void setNozzle(int newNozzle);
    void setType(StateType _type);
    void setDbId(int idFromDb);
    void setControlUser(const std::string& _user);

    static UniversalState getIdleState(ColumnId id);
    virtual ~UniversalState() = default;
};


#endif //PTS2_0_UNIVERSALSTATE_H
