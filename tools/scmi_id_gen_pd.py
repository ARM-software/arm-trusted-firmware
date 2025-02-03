#!/usr/bin/python3

def extract_power_domains(filename):
    table = [["Device Name", "String Representation", "SCMI ID"]]
    i = 0
    with open(filename, 'r') as file:
        for line in file:
            if "AM62L_POWER_DOMAIN" in line:
                line1 = line.split('(')
            else:
                continue
            if (len(line1) > 2):
                line2 = line1[1].rstrip(", str")
                line3 = line1[2][:-4]
                table.append([line2, line3, str(i)])
                i += 1

    return table

def print_table(table):
    col_widths = [max(len(row[i]) for row in table) for i in range(len(table[0]))]

    # Print the table header separator
    print("+" + "+".join("-" * (w + 2) for w in col_widths) + "+")

    for i, row in enumerate(table):
        # Print the row content
        print("| " + " | ".join(cell.ljust(col_widths[j]) for j, cell in enumerate(row)) + " |")

        # Print the separator after the header and after every row
        print("+" + "+".join("-" * (w + 2) for w in col_widths) + "+")

if __name__ == "__main__":
    filename = "plat/ti/k3/board/am62l/scmi/scmi_pd_data.h"  # Change this to your actual C file name
    table = extract_power_domains(filename)
    print_table(table)

