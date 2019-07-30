using System;
using System.Threading;

namespace AppToBeInstrumented
{
    class Program
    {
        static void Main(string[] args)
        {
            while (true)
            {
                new Class1().TestMethodForReJIT();
                Thread.Sleep(2000);
            }
        }
    }

    public class Class1
    {
        public void TestMethodForReJIT()
        {
            Console.WriteLine(DateTime.Now);
        }
    }
}
