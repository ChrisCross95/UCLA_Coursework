// This program accepts as input the meter readings at the
// start and the end of the billing period, the customer's name,
// and the month that the end reading was taken, and returns a water bill.

#include <iostream>
using namespace std;

int initial_Reading;
int final_Reading;
string customer_Name;
int month_Number;
double bill;

int main() {
    
    cout.setf(ios::fixed);
    cout.setf(ios::showpoint);
    cout.precision(2);         // Sets the number of decimal points to 2.

// Lines 17-28 collect information on initial and final meter readings,
// customer name, and month.
    
    cout << "Initial meter reading: ";
    cin >> initial_Reading;
    
    cout << "Final meter reading: ";
    cin >> final_Reading;
    cin.ignore(10000, '\n');
    
    cout << "Customer name: ";
    getline(cin, customer_Name);
    
    cout << "Month number (1=Jan, 2=Feb, etc.)";
    cin >> month_Number;
    
//The following compound if-statement checks to make sure that only valid infromation was
// entered by the user.
    
    if (initial_Reading < 0)
    {
        cout << "---\n";
        cout << "The initial meter reading must be nonnegative.";
    }
        else if (final_Reading < initial_Reading)
        {
            cout << "---\n";
            cout << "The final meter reading must be at least as large as the initial reading.";
        }
        else if (customer_Name == "")
        {
            cout << "---\n";
            cout << "You must enter a customer name.";
        }
        else if (month_Number < 1 || month_Number > 12)
        {
            cout << "---\n";
            cout << "The month number must be in the range 1 through 12.";
        }
    
        // This section calculates the water bill based on whether it is high or low season, and
        // the total cubic feet of water consumed.
        else
        {
            int HCF = final_Reading - initial_Reading;
            
            if (month_Number >= 4 && month_Number <= 10)
            {
                if (HCF > 43)
                {
                    bill = 2.71*43 + 3.39*(HCF - 43);
                    cout << "---\n";
                    cout << "The bill for " << customer_Name;
                    cout << " is $" << bill << endl;
                }
                if (HCF <= 43)
                {
                    bill = 2.71*HCF;
                    cout << "---\n";
                    cout << "The bill for " << customer_Name;
                    cout << " is $" << bill << endl;
                }
            }
            if (month_Number < 4 || month_Number > 10)
            {
                if (HCF > 43)
                {
                    bill = 2.71*43 + 2.87*(HCF - 43);
                    cout << "---\n";
                    cout << "The bill for " << customer_Name;
                    cout << " is $" << bill << endl;
                }
                if (HCF <= 43)
                {
                    bill = 2.71*HCF;
                    cout << "---\n";
                    cout << "The bill for " << customer_Name;
                    cout << " is $" << bill << endl;
                }
        }
    }
    cout << endl;
}
