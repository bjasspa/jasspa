import java.awt.*;
import java.applet.*;

/**
 * This class reads PARAM tags from its HTML host page and sets
 * the color and label properties of the applet. Program execution
 * begins with the init() method. 
 */
public class Applet1 extends Applet
{
	/**
	 * The entry point for the applet. 
	 */
	public void init()
	{
		initForm();

		usePageParams();

		// TODO: Add any constructor code after initForm call.
	}

	private	final String labelParam = "label";
	private	final String backgroundParam = "background";
	private	final String foregroundParam = "foreground";

	/**
	 * Reads parameters from the applet's HTML host and sets applet
	 * properties.
	 */
	private void usePageParams()
	{
		final String defaultLabel = "Default label";
		final String defaultBackground = "C0C0C0";
		final String defaultForeground = "000000";
		String labelValue;
		String backgroundValue;
		String foregroundValue;

		/** 
		 * Read the <PARAM NAME="label" VALUE="some string">,
		 * <PARAM NAME="background" VALUE="rrggbb">,
		 * and <PARAM NAME="foreground" VALUE="rrggbb"> tags from
		 * the applet's HTML host.
		 */
		labelValue = getParameter(labelParam);
		backgroundValue = getParameter(backgroundParam);
		foregroundValue = getParameter(foregroundParam);

		if ((labelValue == null) || (backgroundValue == null) ||
			(foregroundValue == null))
		{
			/**
			 * There was something wrong with the HTML host tags.
			 * Generate default values.
			 */
			labelValue = defaultLabel;
			backgroundValue = defaultBackground;
			foregroundValue = defaultForeground;
		}

		/**
		 * Set the applet's string label, background color, and
		 * foreground colors.
		 */
		label1.setText(labelValue);
		label1.setBackground(stringToColor(backgroundValue));
		label1.setForeground(stringToColor(foregroundValue));
		this.setBackground(stringToColor(backgroundValue));
		this.setForeground(stringToColor(foregroundValue));
	}

	/**
	 * Converts a string formatted as "rrggbb" to an awt.Color object
	 */
	private Color stringToColor(String paramValue)
	{
		int red;
		int green;
		int blue;

		red = (Integer.decode("0x" + paramValue.substring(0,2))).intValue();
		green = (Integer.decode("0x" + paramValue.substring(2,4))).intValue();
		blue = (Integer.decode("0x" + paramValue.substring(4,6))).intValue();

		return new Color(red,green,blue);
	}

	/**
	 * External interface used by design tools to show properties of an applet.
	 */
	public String[][] getParameterInfo()
	{
		String[][] info =
		{
			{ labelParam, "String", "Label string to be displayed" },
			{ backgroundParam, "String", "Background color, format \"rrggbb\"" },
			{ foregroundParam, "String", "Foreground color, format \"rrggbb\"" },
		};
		return info;
	}

	Label label1 = new Label();

	/**
	 * Intializes values for the applet and its components
	 */
	void initForm()
	{
		this.setBackground(Color.lightGray);
		this.setForeground(Color.black);
		label1.setText("label1");
		this.setLayout(new BorderLayout());
		this.add("North",label1);
	}
}
