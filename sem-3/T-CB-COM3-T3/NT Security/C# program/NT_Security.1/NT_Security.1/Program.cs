using System;
using System.Text;

namespace NT_Security._1
{
    class Program
    {
        static void Main(string[] args)
        {
            for(int i = 0; i<args.Length; i++)
            {
                Console.WriteLine($"item {i} is {args[i]}.");
                
            }

            byte[] read = System.IO.File.ReadAllBytes(args[0]);
            byte[] password = Encoding.ASCII.GetBytes(args[1]);

            for(int i = 0; i<read.Length; i++)
            {
                read[i] = (byte)(read[i] ^ password[i % password.Length]);
            }

            System.IO.File.WriteAllBytes(args[2], read);
        }


    }
}
