import 'package:flutter/material.dart';
import 'package:supabase_flutter/supabase_flutter.dart';
import 'package:fl_chart/fl_chart.dart';

const String SUPABASE_URL = 'https://yjjpgvsycxlaqubvedoa.supabase.co';
const String SUPABASE_ANON_KEY =
    'sb_publishable_mf_gFIoOv0-tIu9bhe6fzw_-CYi7BTd';
int riskClass = 0;
String riskLabel = "UNKNOWN";

void main() async {
  WidgetsFlutterBinding.ensureInitialized();

  await Supabase.initialize(url: SUPABASE_URL, anonKey: SUPABASE_ANON_KEY);

  runApp(const MyApp());
}

final supabase = Supabase.instance.client;

class MyApp extends StatelessWidget {
  const MyApp({super.key});

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      debugShowCheckedModeBanner: false,
      theme: ThemeData.dark(),
      home: const Dashboard(),
    );
  }
}

class Dashboard extends StatefulWidget {
  const Dashboard({super.key});

  @override
  State<Dashboard> createState() => _DashboardState();
}

class _DashboardState extends State<Dashboard> {
  // 🌱 plants
  List<String> plants = [];
  String? selectedPlant;
  List<DateTime> timestamps = [];
  String recommendationText = "";
  List<String> actions = [];

  String humidityPref = "mid";
  String temperaturePref = "mid";
  String soilPref = "mid";
  String lightPref = "mid";

  // 📊 metrics
  final metrics = [
    "soil_moisture_pct",
    "temperature_c",
    "humidity_pct",
    "light_level_pct",
  ];

  List<String> selectedMetrics = ["soil_moisture_pct"];

  // ⏱ time range filter
  String timeRange = "24h";

  // 📊 data
  Map<String, List<FlSpot>> graphData = {};

  // 🟩 status
  String statusText = "Loading...";
  Color statusColor = Colors.grey;

  void showTopMessage(String message, Color color) {
    final overlay = Overlay.of(context);

    final entry = OverlayEntry(
      builder: (context) => Positioned(
        top: 50,
        left: 20,
        right: 20,
        child: Material(
          color: Colors.transparent,
          child: Container(
            padding: const EdgeInsets.all(16),
            decoration: BoxDecoration(
              color: color,
              borderRadius: BorderRadius.circular(12),
              boxShadow: [BoxShadow(color: Colors.black26, blurRadius: 10)],
            ),
            child: Text(
              message,
              style: const TextStyle(
                color: Colors.white,
                fontWeight: FontWeight.bold,
              ),
            ),
          ),
        ),
      ),
    );

    overlay.insert(entry);

    Future.delayed(const Duration(seconds: 2), () {
      entry.remove();
    });
  }

  Widget buildDropdown(
    String label,
    String value,
    Function(String?) onChanged,
  ) {
    return Column(
      crossAxisAlignment: CrossAxisAlignment.start,
      children: [
        Text(label),
        DropdownButton<String>(
          value: ["low", "mid", "high"].contains(value) ? value : "mid",
          items: const [
            DropdownMenuItem(value: "low", child: Text("Low")),
            DropdownMenuItem(value: "mid", child: Text("Mid")),
            DropdownMenuItem(value: "high", child: Text("High")),
          ],
          onChanged: onChanged,
        ),
        const SizedBox(height: 10),
      ],
    );
  }

  String normalize(String? v) {
    switch (v) {
      case "pLow":
        return "low";
      case "pMid":
        return "mid";
      case "pHigh":
        return "high";
      default:
        return "mid";
    }
  }

  // =========================
  // LOAD PLANTS
  // =========================
  Future<void> loadPlants() async {
    final res = await supabase.from('plant_settings').select('plant_label');

    final list = (res as List).map((e) => e['plant_label'] as String).toList();

    setState(() {
      plants = list;
      selectedPlant = null;
    });
  }

  // =========================
  // RISK STATUS
  // =========================
  Future<void> loadLatestStatus() async {
    if (selectedPlant == null) return;

    final res = await supabase
        .from('plant_readings')
        .select()
        .eq('plant_label', selectedPlant!)
        .order('timestamp', ascending: false)
        .limit(1);

    if (res.isEmpty) return;

    final latest = res[0];

    final int risk = latest['risk_class'] ?? 0;

    // reset actions
    actions = [];

    // build actions from DB flags
    if (latest['action_reduce_temp'] == true ||
        latest['action_reduce_temp'] == "true" ||
        latest['action_reduce_temp'] == 1) {
      actions.add("❄ Reduce temperature");
    }

    if (latest['action_water'] == true ||
        latest['action_water'] == "true" ||
        latest['action_water'] == 1) {
      actions.add("💧 Water the plant");
    }

    if (latest['action_increase_light'] == true ||
        latest['action_increase_light'] == "true" ||
        latest['action_increase_light'] == 1) {
      actions.add("☀ Increase light exposure");
    }

    setState(() {
      riskClass = risk;

      switch (risk) {
        case 0:
          riskLabel = "HEALTHY";
          statusColor = Colors.green;
          statusText = "Plant is healthy 🌱";
          break;

        case 1:
          riskLabel = "MODERATE RISK";
          statusColor = Colors.orange;
          statusText = "Moderate risk detected ⚠️";
          break;

        case 2:
          riskLabel = "HIGH RISK";
          statusColor = Colors.red;
          statusText = "High risk detected 🚨";
          break;

        default:
          riskLabel = "UNKNOWN";
          statusColor = Colors.grey;
          statusText = "No data";
      }

      recommendationText = actions.isEmpty
          ? "No actions required"
          : actions.join("\n");
    });
  }

  // =========================
  // FILTER TIME RANGE
  // =========================
  bool isInRange(DateTime t) {
    final now = DateTime.now();

    if (timeRange == "24h") {
      return now.difference(t).inHours <= 24;
    }

    if (timeRange == "7d") {
      return now.difference(t).inDays <= 7;
    }

    return true; // all time
  }

  // =========================
  // LOAD GRAPH DATA
  // =========================
  Future<void> loadGraph() async {
    if (selectedPlant == null) return;

    final response = await supabase
        .from('plant_readings')
        .select()
        .eq('plant_label', selectedPlant!)
        .order('timestamp');

    final List<Map<String, dynamic>> data = List<Map<String, dynamic>>.from(
      response,
    );
    Map<String, List<FlSpot>> temp = {};
    timestamps = [];

    final filtered = data.where((row) {
      final t = DateTime.parse(row['timestamp']);
      return isInRange(t);
    }).toList();

    for (int i = 0; i < filtered.length; i++) {
      final row = filtered[i];

      final time = DateTime.parse(row['timestamp']);
      timestamps.add(time);

      for (final m in selectedMetrics) {
        final value = (row[m] ?? 0).toDouble();

        temp.putIfAbsent(m, () => []);
        temp[m]!.add(FlSpot(i.toDouble(), value));
      }
    }

    setState(() {
      graphData = temp;
    });
  }

  Future<void> loadPlantSettings() async {
    if (selectedPlant == null) return;

    final res = await supabase
        .from('plant_settings')
        .select()
        .eq('plant_label', selectedPlant!)
        .limit(1);

    if (res.isEmpty) return;

    final row = res[0];

    setState(() {
      humidityPref = normalize(row['humidity_preference']);
      temperaturePref = normalize(row['temperature_preference']);
      soilPref = normalize(row['soil_preference']);
      lightPref = normalize(row['light_preference']);
    });
  }

  Future<void> savePlantSettings() async {
    if (selectedPlant == null) {
      showTopMessage("❌ No plant selected", Colors.red);
      return;
    }

    final current = await supabase
        .from('plant_settings')
        .select('version')
        .eq('plant_label', selectedPlant!.trim())
        .single();

    final int newVersion = (current['version'] ?? 0) + 1;

    try {
      final response = await supabase
          .from('plant_settings')
          .update({
            'humidity_preference': toDbValue(humidityPref),
            'temperature_preference': toDbValue(temperaturePref),
            'soil_preference': toDbValue(soilPref),
            'light_preference': toDbValue(lightPref),
            'version': newVersion,
          })
          .eq('plant_label', selectedPlant!.trim())
          .select();

      if (response.isEmpty) {
        showTopMessage(
          "⚠️ Nothing updated. Check plant selection.",
          Colors.orange,
        );
        return;
      }

      showTopMessage("✅ Settings saved successfully", Colors.green);
    } catch (_) {
      showTopMessage("❌ Failed to save settings", Colors.red);
    }
  }

  // =========================
  // COLOR MAP
  // =========================
  Color colorFor(String metric) {
    switch (metric) {
      case "soil_moisture_pct":
        return Colors.blue;
      case "temperature_c":
        return Colors.red;
      case "humidity_pct":
        return Colors.green;
      case "light_level_pct":
        return Colors.yellow;
      default:
        return Colors.white;
    }
  }

  String label(String m) {
    switch (m) {
      case "soil_moisture_pct":
        return "Soil Moisture";
      case "temperature_c":
        return "Temperature";
      case "humidity_pct":
        return "Humidity";
      case "light_level_pct":
        return "Light";
      default:
        return m;
    }
  }

  String toDbValue(String v) {
    switch (v) {
      case "low":
        return "pLow";
      case "mid":
        return "pMid";
      case "high":
        return "pHigh";
      default:
        return "pMid";
    }
  }

  // =========================
  // INIT
  // =========================
  @override
  void initState() {
    super.initState();
    loadPlants().then((_) => loadGraph());
    loadLatestStatus();
  }

  // =========================
  // UI
  // =========================
  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(title: Text(selectedPlant ?? "Plant Dashboard")),
      body: Padding(
        padding: const EdgeInsets.all(12),
        child: Column(
          children: [
            // 🌱 plant selector
            DropdownButton<String>(
              value: selectedPlant,
              hint: const Text("Select a plant"),
              items: plants
                  .map((p) => DropdownMenuItem(value: p, child: Text(p)))
                  .toList(),
              onChanged: (v) {
                setState(() => selectedPlant = v);
                loadGraph();
                loadLatestStatus();
                loadPlantSettings();
              },
            ),

            const SizedBox(height: 10),

            // ⏱ TIME RANGE DROPDOWN
            Row(
              children: [
                const Text("Time range: "),
                const SizedBox(width: 10),
                DropdownButton<String>(
                  value: timeRange,
                  items: const [
                    DropdownMenuItem(
                      value: "24h",
                      child: Text("Past 24 hours"),
                    ),
                    DropdownMenuItem(value: "7d", child: Text("Past 7 days")),
                    DropdownMenuItem(value: "all", child: Text("All time")),
                  ],
                  onChanged: (v) {
                    setState(() => timeRange = v!);
                    loadGraph();
                  },
                ),
              ],
            ),

            const SizedBox(height: 10),

            // 🔘 METRIC SELECTOR
            Wrap(
              spacing: 8,
              children: metrics.map((m) {
                final selected = selectedMetrics.contains(m);

                return FilterChip(
                  label: Text(label(m)),
                  selected: selected,
                  onSelected: (v) {
                    setState(() {
                      v ? selectedMetrics.add(m) : selectedMetrics.remove(m);
                    });
                    loadGraph();
                  },
                );
              }).toList(),
            ),

            const SizedBox(height: 10),

            // 📊 LEGEND
            Wrap(
              spacing: 12,
              children: selectedMetrics.map((m) {
                return Row(
                  mainAxisSize: MainAxisSize.min,
                  children: [
                    Container(width: 10, height: 10, color: colorFor(m)),
                    const SizedBox(width: 5),
                    Text(label(m)),
                  ],
                );
              }).toList(),
            ),

            const SizedBox(height: 10),

            // 📊 GRAPH (NO ZOOMING)
            Expanded(
              child: Row(
                children: [
                  // =====================
                  // GRAPH SIDE
                  // =====================
                  Expanded(
                    flex: 3,
                    child: LineChart(
                      LineChartData(
                        minY: 0,
                        maxY: 100,
                        gridData: const FlGridData(show: true),
                        titlesData: FlTitlesData(
                          rightTitles: const AxisTitles(
                            sideTitles: SideTitles(showTitles: false),
                          ),
                          topTitles: const AxisTitles(
                            sideTitles: SideTitles(showTitles: false),
                          ),
                          bottomTitles: AxisTitles(
                            sideTitles: SideTitles(
                              showTitles: true,
                              interval: timestamps.isNotEmpty
                                  ? (timestamps.length / 5).ceilToDouble()
                                  : 1,
                              getTitlesWidget: (value, meta) {
                                final i = value.toInt();
                                if (i < 0 || i >= timestamps.length) {
                                  return const Text("");
                                }
                                final t = timestamps[i];
                                return Text("${t.day}/${t.month}");
                              },
                            ),
                          ),
                          leftTitles: const AxisTitles(
                            sideTitles: SideTitles(
                              showTitles: true,
                              reservedSize: 40,
                            ),
                          ),
                        ),
                        lineBarsData: selectedMetrics.map((m) {
                          return LineChartBarData(
                            spots: graphData[m] ?? [],
                            isCurved: true,
                            barWidth: 2,
                            color: colorFor(m),
                          );
                        }).toList(),
                      ),
                    ),
                  ),

                  const SizedBox(width: 12),

                  // =====================
                  // SETTINGS PANEL
                  // =====================
                  Expanded(
                    flex: 2,
                    child: Container(
                      padding: const EdgeInsets.all(16),
                      decoration: BoxDecoration(
                        color: Colors.white.withOpacity(0.05),
                        borderRadius: BorderRadius.circular(16),
                        border: Border.all(color: Colors.white24),
                      ),
                      child: Column(
                        crossAxisAlignment: CrossAxisAlignment.start,
                        children: [
                          const Text(
                            "Plant Settings",
                            style: TextStyle(
                              fontSize: 18,
                              fontWeight: FontWeight.bold,
                            ),
                          ),

                          const SizedBox(height: 15),

                          buildDropdown("Humidity", humidityPref, (v) {
                            setState(() => humidityPref = v!);
                          }),

                          buildDropdown("Temperature", temperaturePref, (v) {
                            setState(() => temperaturePref = v!);
                          }),

                          buildDropdown("Soil Moisture", soilPref, (v) {
                            setState(() => soilPref = v!);
                          }),

                          buildDropdown("Light", lightPref, (v) {
                            setState(() => lightPref = v!);
                          }),

                          const Spacer(),

                          SizedBox(
                            width: double.infinity,
                            child: ElevatedButton(
                              onPressed: savePlantSettings,
                              child: const Text("Save Settings"),
                            ),
                          ),
                        ],
                      ),
                    ),
                  ),
                ],
              ),
            ),
            Container(
              width: double.infinity,
              padding: const EdgeInsets.all(16),
              decoration: BoxDecoration(
                color: statusColor,
                borderRadius: BorderRadius.circular(12),
              ),
              child: Column(
                crossAxisAlignment: CrossAxisAlignment.start,
                children: [
                  Text(
                    "STATUS: $riskLabel",
                    style: const TextStyle(
                      fontSize: 18,
                      fontWeight: FontWeight.bold,
                    ),
                  ),

                  const SizedBox(height: 5),

                  Text(statusText),

                  const SizedBox(height: 12),

                  const Text(
                    "Recommendation:",
                    style: TextStyle(fontWeight: FontWeight.bold),
                  ),

                  const SizedBox(height: 6),

                  Text(recommendationText),
                ],
              ),
            ),
          ],
        ),
      ),
    );
  }
}
