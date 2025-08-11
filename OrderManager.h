
#include <map>
#include <websocketpp/config/asio_client.hpp>
#include <websocketpp/client.hpp>

// OrderManager class
class OrderManager
{
private:
    std::unordered_map<std::string, Order> orders;
    client *wsClient;
    websocketpp::connection_hdl wsHandle;

    bool orderExists(const std::string &orderId) const
    {
        return orders.find(orderId) != orders.end();
    }

    void sendApiRequest(const std::string &requestJson);

public:
    OrderManager(client *clientPtr, websocketpp::connection_hdl hdl);

    bool placeOrder(const Order &order);

    bool cancelOrder(const std::string &orderId);

    bool modifyOrder(const std::string &orderId, const std::optional<double> &newPrice, const std::optional<double> &newAmount, std::string &advanced, bool &post_only, bool &reduce_only);
    void handleApiResponse(const std::string &response);

    std::optional<Order> getOrderById(const std::string &orderId) const;

    std::vector<Order> getAllOrders() const;

    std::unordered_map<std::string, double> getCurrentPositions() const;

    bool processApiResponse(const std::string &response);

    //  method: Get order history by currency
    void getOrderHistoryByCurrency(const std::string &currency);

    //  method: Get order history by instrument
    void getOrderHistoryByInstrument(const std::string &instrument);

    //  method: Stream market data
    void streamMarketData(const std::string &channel);

    //  method: Get summary by instrument
    void getSummaryByInstrument(const std::string &instrument);

    //  method: Get summary by currency
    void getSummaryByCurrency(const std::string &currency);

    //  method: Get ticker data
    void getTickerData(const std::string &instrument);

    //  method: Get contract size
    void getContractSize(const std::string &instrument);

    //  method: Get all supported currencies
    void getAllSupportedCurrencies();

    //  method: Subscribe to a channel
    void subscribe(const std::string &channel);

    // New method: Unsubscribe from a channel
    void unsubscribe(const std::string &channel);

    // New method: Unsubscribe from all channels
    void unsubscribeAll();
};